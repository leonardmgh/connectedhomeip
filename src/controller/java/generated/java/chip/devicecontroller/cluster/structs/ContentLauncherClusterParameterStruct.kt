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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter
import java.util.Optional

class ContentLauncherClusterParameterStruct(
  val type: Int,
  val value: String,
  val externalIDList: Optional<List<ContentLauncherClusterAdditionalInfoStruct>>
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterParameterStruct {\n")
    append("\ttype : $type\n")
    append("\tvalue : $value\n")
    append("\texternalIDList : $externalIDList\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_TYPE), type)
      put(ContextSpecificTag(TAG_VALUE), value)
      if (externalIDList.isPresent) {
        val optexternalIDList = externalIDList.get()
        startList(ContextSpecificTag(TAG_EXTERNAL_I_D_LIST))
        for (item in optexternalIDList.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endList()
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TYPE = 0
    private const val TAG_VALUE = 1
    private const val TAG_EXTERNAL_I_D_LIST = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ContentLauncherClusterParameterStruct {
      tlvReader.enterStructure(tag)
      val type = tlvReader.getInt(ContextSpecificTag(TAG_TYPE))
      val value = tlvReader.getString(ContextSpecificTag(TAG_VALUE))
      val externalIDList =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_EXTERNAL_I_D_LIST))) {
          Optional.of(
            buildList<ContentLauncherClusterAdditionalInfoStruct> {
              tlvReader.enterList(ContextSpecificTag(TAG_EXTERNAL_I_D_LIST))
              while (!tlvReader.isEndOfContainer()) {
                add(ContentLauncherClusterAdditionalInfoStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ContentLauncherClusterParameterStruct(type, value, externalIDList)
    }
  }
}
