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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class UnitTestingClusterTestEventEvent(
  val arg1: Int,
  val arg2: Int,
  val arg3: Boolean,
  val arg4: chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct,
  val arg5: List<chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct>,
  val arg6: List<Int>
) {
  override fun toString(): String = buildString {
    append("UnitTestingClusterTestEventEvent {\n")
    append("\targ1 : $arg1\n")
    append("\targ2 : $arg2\n")
    append("\targ3 : $arg3\n")
    append("\targ4 : $arg4\n")
    append("\targ5 : $arg5\n")
    append("\targ6 : $arg6\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ARG1), arg1)
      put(ContextSpecificTag(TAG_ARG2), arg2)
      put(ContextSpecificTag(TAG_ARG3), arg3)
      arg4.toTlv(ContextSpecificTag(TAG_ARG4), this)
      startList(ContextSpecificTag(TAG_ARG5))
      for (item in arg5.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endList()
      startList(ContextSpecificTag(TAG_ARG6))
      for (item in arg6.iterator()) {
        put(AnonymousTag, item)
      }
      endList()
      endStructure()
    }
  }

  companion object {
    private const val TAG_ARG1 = 1
    private const val TAG_ARG2 = 2
    private const val TAG_ARG3 = 3
    private const val TAG_ARG4 = 4
    private const val TAG_ARG5 = 5
    private const val TAG_ARG6 = 6

    fun fromTlv(tag: Tag, tlvReader: TlvReader): UnitTestingClusterTestEventEvent {
      tlvReader.enterStructure(tag)
      val arg1 = tlvReader.getInt(ContextSpecificTag(TAG_ARG1))
      val arg2 = tlvReader.getInt(ContextSpecificTag(TAG_ARG2))
      val arg3 = tlvReader.getBoolean(ContextSpecificTag(TAG_ARG3))
      val arg4 =
        chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct.fromTlv(
          ContextSpecificTag(TAG_ARG4),
          tlvReader
        )
      val arg5 =
        buildList<chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct> {
          tlvReader.enterList(ContextSpecificTag(TAG_ARG5))
          while (!tlvReader.isEndOfContainer()) {
            this.add(
              chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct.fromTlv(
                AnonymousTag,
                tlvReader
              )
            )
          }
          tlvReader.exitContainer()
        }
      val arg6 =
        buildList<Int> {
          tlvReader.enterList(ContextSpecificTag(TAG_ARG6))
          while (!tlvReader.isEndOfContainer()) {
            this.add(tlvReader.getInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return UnitTestingClusterTestEventEvent(arg1, arg2, arg3, arg4, arg5, arg6)
    }
  }
}
