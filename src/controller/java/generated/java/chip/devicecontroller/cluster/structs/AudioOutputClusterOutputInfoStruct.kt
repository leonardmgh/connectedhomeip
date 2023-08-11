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

class AudioOutputClusterOutputInfoStruct(val index: Int, val outputType: Int, val name: String) {
  override fun toString(): String = buildString {
    append("AudioOutputClusterOutputInfoStruct {\n")
    append("\tindex : $index\n")
    append("\toutputType : $outputType\n")
    append("\tname : $name\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_INDEX), index)
      put(ContextSpecificTag(TAG_OUTPUT_TYPE), outputType)
      put(ContextSpecificTag(TAG_NAME), name)
      endStructure()
    }
  }

  companion object {
    private const val TAG_INDEX = 0
    private const val TAG_OUTPUT_TYPE = 1
    private const val TAG_NAME = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader): AudioOutputClusterOutputInfoStruct {
      tlvReader.enterStructure(tag)
      val index = tlvReader.getInt(ContextSpecificTag(TAG_INDEX))
      val outputType = tlvReader.getInt(ContextSpecificTag(TAG_OUTPUT_TYPE))
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))

      tlvReader.exitContainer()

      return AudioOutputClusterOutputInfoStruct(index, outputType, name)
    }
  }
}
