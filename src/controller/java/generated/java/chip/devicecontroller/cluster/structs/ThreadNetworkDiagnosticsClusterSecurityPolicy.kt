/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class ThreadNetworkDiagnosticsClusterSecurityPolicy(val rotationTime: Int, val flags: Int) {
  override fun toString(): String = buildString {
    append("ThreadNetworkDiagnosticsClusterSecurityPolicy {\n")
    append("\trotationTime : $rotationTime\n")
    append("\tflags : $flags\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ROTATION_TIME), rotationTime)
      put(ContextSpecificTag(TAG_FLAGS), flags)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ROTATION_TIME = 0
    private const val TAG_FLAGS = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ThreadNetworkDiagnosticsClusterSecurityPolicy {
      tlvReader.enterStructure(tag)
      val rotationTime = tlvReader.getInt(ContextSpecificTag(TAG_ROTATION_TIME))
      val flags = tlvReader.getInt(ContextSpecificTag(TAG_FLAGS))

      tlvReader.exitContainer()

      return ThreadNetworkDiagnosticsClusterSecurityPolicy(rotationTime, flags)
    }
  }
}
