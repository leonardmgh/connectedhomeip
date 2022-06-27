/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

// THIS FILE IS GENERATED BY ZAP

#pragma once

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model/DecodableList.h>
#include <app/util/af-enums.h>
#include <app/util/attribute-filter.h>
#include <app/util/im-client-callbacks.h>
#include <inttypes.h>
#include <lib/support/FunctionTraits.h>
#include <lib/support/Span.h>

// List specific responses
void BindingClusterBindingListAttributeFilter(chip::TLV::TLVReader * data, chip::Callback::Cancelable * onSuccessCallback,
                                              chip::Callback::Cancelable * onFailureCallback);
typedef void (*BindingBindingListAttributeCallback)(
    void * context,
    const chip::app::DataModel::DecodableList<chip::app::Clusters::Binding::Structs::TargetStruct::DecodableType> & data);
void BindingClusterGeneratedCommandListListAttributeFilter(chip::TLV::TLVReader * data,
                                                           chip::Callback::Cancelable * onSuccessCallback,
                                                           chip::Callback::Cancelable * onFailureCallback);
typedef void (*BindingGeneratedCommandListListAttributeCallback)(void * context,
                                                                 const chip::app::DataModel::DecodableList<chip::CommandId> & data);
void BindingClusterAcceptedCommandListListAttributeFilter(chip::TLV::TLVReader * data,
                                                          chip::Callback::Cancelable * onSuccessCallback,
                                                          chip::Callback::Cancelable * onFailureCallback);
typedef void (*BindingAcceptedCommandListListAttributeCallback)(void * context,
                                                                const chip::app::DataModel::DecodableList<chip::CommandId> & data);
void BindingClusterAttributeListListAttributeFilter(chip::TLV::TLVReader * data, chip::Callback::Cancelable * onSuccessCallback,
                                                    chip::Callback::Cancelable * onFailureCallback);
typedef void (*BindingAttributeListListAttributeCallback)(void * context,
                                                          const chip::app::DataModel::DecodableList<chip::AttributeId> & data);
void OnOffClusterGeneratedCommandListListAttributeFilter(chip::TLV::TLVReader * data,
                                                         chip::Callback::Cancelable * onSuccessCallback,
                                                         chip::Callback::Cancelable * onFailureCallback);
typedef void (*OnOffGeneratedCommandListListAttributeCallback)(void * context,
                                                               const chip::app::DataModel::DecodableList<chip::CommandId> & data);
void OnOffClusterAcceptedCommandListListAttributeFilter(chip::TLV::TLVReader * data, chip::Callback::Cancelable * onSuccessCallback,
                                                        chip::Callback::Cancelable * onFailureCallback);
typedef void (*OnOffAcceptedCommandListListAttributeCallback)(void * context,
                                                              const chip::app::DataModel::DecodableList<chip::CommandId> & data);
void OnOffClusterAttributeListListAttributeFilter(chip::TLV::TLVReader * data, chip::Callback::Cancelable * onSuccessCallback,
                                                  chip::Callback::Cancelable * onFailureCallback);
typedef void (*OnOffAttributeListListAttributeCallback)(void * context,
                                                        const chip::app::DataModel::DecodableList<chip::AttributeId> & data);
void ScenesClusterGeneratedCommandListListAttributeFilter(chip::TLV::TLVReader * data,
                                                          chip::Callback::Cancelable * onSuccessCallback,
                                                          chip::Callback::Cancelable * onFailureCallback);
typedef void (*ScenesGeneratedCommandListListAttributeCallback)(void * context,
                                                                const chip::app::DataModel::DecodableList<chip::CommandId> & data);
void ScenesClusterAcceptedCommandListListAttributeFilter(chip::TLV::TLVReader * data,
                                                         chip::Callback::Cancelable * onSuccessCallback,
                                                         chip::Callback::Cancelable * onFailureCallback);
typedef void (*ScenesAcceptedCommandListListAttributeCallback)(void * context,
                                                               const chip::app::DataModel::DecodableList<chip::CommandId> & data);
void ScenesClusterAttributeListListAttributeFilter(chip::TLV::TLVReader * data, chip::Callback::Cancelable * onSuccessCallback,
                                                   chip::Callback::Cancelable * onFailureCallback);
typedef void (*ScenesAttributeListListAttributeCallback)(void * context,
                                                         const chip::app::DataModel::DecodableList<chip::AttributeId> & data);
