/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      Declaration of Commissioner Discovery Controller,
 *      a common class that manages state and callbacks
 *      for handling the Commissioner Discovery
 *      and User Directed Commissioning workflow
 *
 */
#include <controller/CommissionerDiscoveryController.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

using namespace ::chip;
using namespace chip::Protocols::UserDirectedCommissioning;

void CommissionerDiscoveryController::ResetState()
{
    mCurrentInstance[0] = '\0';
    mVendorId           = 0;
    mProductId          = 0;
    mNodeId             = 0;
    mReady              = true;
}

void CommissionerDiscoveryController::OnUserDirectedCommissioningRequest(UDCClientState state)
{
    if (!mReady)
    {
        ChipLogDetail(Controller, "CommissionerDiscoveryController not read. Current instance=%s", mCurrentInstance);
        return;
    }
    // first check if this is a cancel
    if (state.GetCancelPasscode())
    {
        ChipLogDetail(Controller, "------PROMPT USER: %s cancelled commissioning [" ChipLogFormatMEI "," ChipLogFormatMEI ",%s]",
                      state.GetDeviceName(), ChipLogValueMEI(state.GetVendorId()), ChipLogValueMEI(state.GetProductId()),
                      state.GetInstanceName());
        if (mUserPrompter != nullptr)
        {
            mUserPrompter->HidePromptsOnCancel(state.GetVendorId(), state.GetProductId(), state.GetDeviceName());
        }
        return;
    }
    if (state.GetCommissionerPasscodeReady() && state.GetCdPort() != 0)
    {
        uint32_t passcode = state.GetCachedCommissionerPasscode();
        if (!mReady || passcode == 0)
        {
            ChipLogError(AppServer, "On UDC: commissioner passcode ready but no passcode");
            CommissionerDeclaration cd;
            cd.SetErrorCode(CommissionerDeclaration::CdError::kUnexpectedCommissionerPasscodeReady);

            if (mUdcServer == nullptr)
            {
                ChipLogError(AppServer, "On UDC: no udc server");
                return;
            }
            mUdcServer->SendCDCMessage(cd,
                                       chip::Transport::PeerAddress::UDP(state.GetPeerAddress().GetIPAddress(), state.GetCdPort()));
            return;
        }
        else
        {
            // can only get here is ok() has already been called
            ChipLogError(AppServer, "On UDC: commissioner passcode ready with passcode - commissioning");

            // start commissioning using the cached passcode
            CommissionWithPasscode(passcode);
            return;
        }
    }

    mReady = false;
    Platform::CopyString(mCurrentInstance, state.GetInstanceName());
    mPendingConsent = true;
    char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1];
    Encoding::BytesToUppercaseHexString(state.GetRotatingId(), state.GetRotatingIdLength(), rotatingIdString,
                                        sizeof(rotatingIdString));

    ChipLogDetail(Controller,
                  "------PROMPT USER: %s is requesting permission to cast to this TV, approve? [" ChipLogFormatMEI
                  "," ChipLogFormatMEI ",%s,%s]",
                  state.GetDeviceName(), ChipLogValueMEI(state.GetVendorId()), ChipLogValueMEI(state.GetProductId()),
                  state.GetInstanceName(), rotatingIdString);
    if (mUserPrompter != nullptr)
    {
        mUserPrompter->PromptForCommissionOKPermission(state.GetVendorId(), state.GetProductId(), state.GetDeviceName());
    }
    ChipLogDetail(Controller, "------Via Shell Enter: controller ux ok|cancel");
}

void CommissionerDiscoveryController::Ok()
{
    chip::DeviceLayer::StackLock lock;

    if (!mPendingConsent)
    {
        ChipLogError(AppServer, "UX Ok: no current instance");
        return;
    }
    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX Ok: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX Ok: could not find instance=%s", mCurrentInstance);
        return;
    }
    if (client->GetUDCClientProcessingState() != UDCClientProcessingState::kPromptingUser)
    {
        ChipLogError(AppServer, "UX Ok: invalid state for ok");
        return;
    }
    client->SetUDCClientProcessingState(UDCClientProcessingState::kObtainingOnboardingPayload);

    if (mPasscodeService == nullptr)
    {
        HandleContentAppPasscodeResponse(0);
        return;
    }

    char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
    Encoding::BytesToUppercaseHexString(client->GetRotatingId(), client->GetRotatingIdLength(), rotatingIdString,
                                        sizeof(rotatingIdString));
    CharSpan rotatingIdSpan = chip::CharSpan(rotatingIdString, sizeof(rotatingIdString));

    uint8_t targetAppCount = client->GetNumTargetAppInfos();
    if (targetAppCount > 0)
    {
        for (uint8_t i = 0; i < targetAppCount; i++)
        {
            TargetAppInfo info;
            if (client->GetTargetAppInfo(i, info))
            {
                mPasscodeService->LookupTargetContentApp(client->GetVendorId(), client->GetProductId(), rotatingIdSpan, info);
            }
        }
        return;
    }

    mPasscodeService->FetchCommissionPasscodeFromContentApp(client->GetVendorId(), client->GetProductId(), rotatingIdSpan);
    return;
}

void CommissionerDiscoveryController::HandleTargetContentAppCheck(TargetAppInfo target, uint32_t passcode)
{
    chip::DeviceLayer::StackLock lock;

    bool foundTargetApp      = false;
    bool foundPendingTargets = false;

    /**
     * Update our target app list with the status from this target.
     *
     * If we are the first callback to receive a passcode,
     *  then complete commissioning with it.
     * If we are the last expected callback and none has completed commissioning,
     *  then advance to the next step for trying to obtain a passcode.
     * When iterating through the list of targets, keep track of whether any apps have been found,
     *  so that if we advance we can do so with that information (may need to send a CDC).
     */

    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppCheck: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppCheck: could not find instance=%s", mCurrentInstance);
        return;
    }
    if (client->GetUDCClientProcessingState() != UDCClientProcessingState::kObtainingOnboardingPayload)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppCheck: invalid state for HandleContentAppPasscodeResponse");
        return;
    }

    uint8_t targetAppCount = client->GetNumTargetAppInfos();
    for (uint8_t i = 0; i < targetAppCount; i++)
    {
        TargetAppInfo info;
        if (client->GetTargetAppInfo(i, info))
        {
            if (info.checkState == TargetAppCheckState::kAppFoundPasscodeReturned)
            {
                // nothing else to do, complete commissioning has been called
                return;
            }
            else if (info.checkState == TargetAppCheckState::kAppFoundNoPasscode)
            {
                foundTargetApp = true;
            }
            else if (info.checkState == TargetAppCheckState::kNotInitialized)
            {
                if (target.vendorId == info.vendorId && target.productId == info.productId)
                {
                    client->SetTargetAppInfoState(i, target.checkState);
                    if (target.checkState != TargetAppCheckState::kAppNotFound)
                    {
                        foundTargetApp = true;
                    }
                }
                else
                {
                    foundPendingTargets = true;
                }
            }
        }
    }
    if (passcode != 0)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppCheck: found a passcode");
        // we found a passcode and complete commissioning has not been called
        CommissionWithPasscode(passcode);
        return;
    }
    if (foundPendingTargets)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppCheck: have not heard from all apps");
        // have not heard from all targets so don't do anything
        return;
    }
    if (!foundTargetApp)
    {
        // finished iterating through all apps and found none, send CDC
        ChipLogError(AppServer, "UX Ok - HandleContentAppCheck: target apps specified but none found, sending CDC");
        CommissionerDeclaration cd;
        cd.SetNoAppsFound(true);
        mUdcServer->SendCDCMessage(cd,
                                   chip::Transport::PeerAddress::UDP(client->GetPeerAddress().GetIPAddress(), client->GetCdPort()));
    }
    ChipLogError(AppServer, "UX Ok - HandleContentAppCheck: advancing");
    // otherwise, advance to the next step for trying to obtain a passcode.
    HandleContentAppPasscodeResponse(0);
    return;
}

void CommissionerDiscoveryController::HandleContentAppPasscodeResponse(uint32_t passcode)
{
    chip::DeviceLayer::StackLock lock;

    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppPasscodeResponse: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppPasscodeResponse: could not find instance=%s", mCurrentInstance);
        return;
    }
    if (client->GetUDCClientProcessingState() != UDCClientProcessingState::kObtainingOnboardingPayload)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppPasscodeResponse: invalid state for HandleContentAppPasscodeResponse");
        return;
    }

    if (mPasscodeService != nullptr)
    {
        // if CommissionerPasscode
        //    - if CommissionerPasscodeReady, then start commissioning
        //    - if CommissionerPasscode, then call new UX method to show passcode, send CDC
        if (passcode == 0 && client->GetCommissionerPasscode() && client->GetCdPort() != 0)
        {
            char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
            Encoding::BytesToUppercaseHexString(client->GetRotatingId(), client->GetRotatingIdLength(), rotatingIdString,
                                                sizeof(rotatingIdString));
            CharSpan rotatingIdSpan = chip::CharSpan(rotatingIdString, sizeof(rotatingIdString));

            // first step of commissioner passcode
            ChipLogError(AppServer, "UX Ok: commissioner passcode, sending CDC");
            // generate a passcode
            passcode = mPasscodeService->GetCommissionerPasscode(client->GetVendorId(), client->GetProductId(), rotatingIdSpan);
            if (passcode == 0)
            {
                // passcode feature disabled
                ChipLogError(AppServer, "UX Ok: commissioner passcode disabled, sending CDC with error");
                CommissionerDeclaration cd;
                cd.SetErrorCode(CommissionerDeclaration::CdError::kCommissionerPasscodeDisabled);
                cd.SetNeedsPasscode(true);
                mUdcServer->SendCDCMessage(
                    cd, chip::Transport::PeerAddress::UDP(client->GetPeerAddress().GetIPAddress(), client->GetCdPort()));
                return;
            }
            client->SetCachedCommissionerPasscode(passcode);

            CommissionerDeclaration cd;
            cd.SetCommissionerPasscode(true);
            if (mUserPrompter->DisplaysPasscodeAndQRCode())
            {
                cd.SetQRCodeDisplayed(true);
            }
            mUdcServer->SendCDCMessage(
                cd, chip::Transport::PeerAddress::UDP(client->GetPeerAddress().GetIPAddress(), client->GetCdPort()));

            // dialog
            ChipLogDetail(Controller,
                          "------PROMPT USER: %s is requesting permission to cast to this TV. Casting passcode: [" ChipLogFormatMEI
                          "]. Additional instructions [" ChipLogFormatMEI "] [%s]. [" ChipLogFormatMEI "," ChipLogFormatMEI ",%s]",
                          client->GetDeviceName(), ChipLogValueMEI(passcode), ChipLogValueMEI(client->GetPairingHint()),
                          client->GetPairingInst(), ChipLogValueMEI(client->GetVendorId()), ChipLogValueMEI(client->GetProductId()),
                          client->GetInstanceName());
            mUserPrompter->PromptWithCommissionerPasscode(client->GetVendorId(), client->GetProductId(), client->GetDeviceName(),
                                                          passcode, client->GetPairingHint(), client->GetPairingInst());
            return;
        }
        if (passcode != 0)
        {
            CommissionWithPasscode(passcode);
            return;
        }
    }

    // if NoPasscode, send CDC
    if (client->GetNoPasscode() && client->GetCdPort() != 0)
    {
        ChipLogError(AppServer, "UX Ok: no app passcode and NoPasscode in UDC, sending CDC");
        CommissionerDeclaration cd;
        cd.SetNeedsPasscode(true);
        mUdcServer->SendCDCMessage(cd,
                                   chip::Transport::PeerAddress::UDP(client->GetPeerAddress().GetIPAddress(), client->GetCdPort()));
        return;
    }

    // if CdUponPasscodeDialog, send CDC
    if (client->GetCdUponPasscodeDialog() && client->GetCdPort() != 0)
    {
        ChipLogError(AppServer, "UX Ok: no app passcode and GetCdUponPasscodeDialog in UDC, sending CDC");
        CommissionerDeclaration cd;
        cd.SetNeedsPasscode(true); // TODO: should this be set?
        cd.SetPasscodeDialogDisplayed(true);
        mUdcServer->SendCDCMessage(cd,
                                   chip::Transport::PeerAddress::UDP(client->GetPeerAddress().GetIPAddress(), client->GetCdPort()));
    }

    ChipLogDetail(Controller, "------PROMPT USER: please enter passcode displayed in casting app ");
    if (mUserPrompter != nullptr)
    {
        mUserPrompter->PromptForCommissionPasscode(client->GetVendorId(), client->GetProductId(), client->GetDeviceName(),
                                                   client->GetPairingHint(), client->GetPairingInst());
    }
    ChipLogDetail(Controller, "------Via Shell Enter: controller ux ok [passcode]");
}

void CommissionerDiscoveryController::CommissionWithPasscode(uint32_t passcode)
{
    chip::DeviceLayer::StackLock lock;

    if (!mPendingConsent)
    {
        ChipLogError(AppServer, "UX Cancel: no current instance");
        return;
    }
    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX CommissionWithPasscode: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX Ok: could not find instance=%s", mCurrentInstance);
        return;
    }
    // state needs to be either kPromptingUser or kObtainingOnboardingPayload
    if (!(client->GetUDCClientProcessingState() == UDCClientProcessingState::kPromptingUser ||
          client->GetUDCClientProcessingState() == UDCClientProcessingState::kObtainingOnboardingPayload))
    {
        ChipLogError(AppServer, "UX CommissionWithPasscode: invalid state for CommissionWithPasscode");
        return;
    }
    Transport::PeerAddress peerAddress = client->GetPeerAddress();
    client->SetUDCClientProcessingState(UDCClientProcessingState::kCommissioningNode);
    if (mCommissionerCallback != nullptr)
    {
        if (mUserPrompter != nullptr)
        {
            mUserPrompter->PromptCommissioningStarted(client->GetVendorId(), client->GetProductId(), client->GetDeviceName());
        }
        mCommissionerCallback->ReadyForCommissioning(passcode, client->GetLongDiscriminator(), peerAddress);
    }
}

void CommissionerDiscoveryController::Cancel()
{
    chip::DeviceLayer::StackLock lock;

    if (!mPendingConsent)
    {
        ChipLogError(AppServer, "UX Cancel: no current instance");
        return;
    }
    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX Cancel: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr || client->GetUDCClientProcessingState() != UDCClientProcessingState::kPromptingUser)
    {
        ChipLogError(AppServer, "UX Cancel: invalid state for cancel");
        return;
    }
    client->SetUDCClientProcessingState(UDCClientProcessingState::kUserDeclined);
    mPendingConsent = false;
}

void CommissionerDiscoveryController::CommissioningSucceeded(uint16_t vendorId, uint16_t productId, NodeId nodeId,
                                                             Messaging::ExchangeManager & exchangeMgr,
                                                             const SessionHandle & sessionHandle)
{
    mVendorId  = vendorId;
    mProductId = productId;
    mNodeId    = nodeId;
    if (mPostCommissioningListener != nullptr)
    {
        ChipLogDetail(Controller, "CommissionerDiscoveryController calling listener");
        mPostCommissioningListener->CommissioningCompleted(vendorId, productId, nodeId, exchangeMgr, sessionHandle);
    }
    else
    {
        PostCommissioningSucceeded();
    }
}

void CommissionerDiscoveryController::CommissioningFailed(CHIP_ERROR error)
{
    if (mUserPrompter != nullptr)
    {
        ChipLogDetail(Controller, "------PROMPT USER: commissioning failed ");
        mUserPrompter->PromptCommissioningFailed(GetCommissioneeName(), error);
    }
    ResetState();
}

void CommissionerDiscoveryController::PostCommissioningSucceeded()
{
    if (mUserPrompter != nullptr)
    {
        ChipLogDetail(Controller, "------PROMPT USER: commissioning success ");
        mUserPrompter->PromptCommissioningSucceeded(mVendorId, mProductId, GetCommissioneeName());
    }
    ResetState();
}

void CommissionerDiscoveryController::PostCommissioningFailed(CHIP_ERROR error)
{
    if (mUserPrompter != nullptr)
    {
        ChipLogDetail(Controller, "------PROMPT USER: post-commissioning failed ");
        mUserPrompter->PromptCommissioningFailed(GetCommissioneeName(), error);
    }
    ResetState();
}

const char * CommissionerDiscoveryController::GetCommissioneeName()
{
    if (mReady)
    {
        // no current commissionee
        ChipLogError(AppServer, "CommissionerDiscoveryController no current commissionee");
        return nullptr;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "CommissionerDiscoveryController no UDCState for instance=%s", mCurrentInstance);
        return nullptr;
    }
    return client->GetDeviceName();
}

UDCClientState * CommissionerDiscoveryController::GetUDCClientState()
{
    if (mReady)
    {
        // no current commissionee
        ChipLogError(AppServer, "CommissionerDiscoveryController no current commissionee");
        return nullptr;
    }
    return mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
