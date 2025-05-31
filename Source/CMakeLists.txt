#-------------------------------------------------------------------------------
#  Copyright (c) 2025 John D. Haughton
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#-------------------------------------------------------------------------------

function(autogen_py NAME)

   add_custom_command(
      COMMENT
         "Auto-generate ${NAME}"
      COMMAND
         ${CMAKE_COMMAND}
         -E env PYTHONPATH=${CMAKE_SOURCE_DIR}/Platform/python
         python3 ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.py
      DEPENDS
         ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.py
      OUTPUT
         ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.h
         ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cpp
     )

endfunction()

add_subdirectory(hw)
add_subdirectory(chip)
add_subdirectory(VGM)

foreach(hw_config ${hw_config_list})

   add_executable(picoChippy_${hw_config} picoChippy.cpp)

   target_compile_definitions(picoChippy_${hw_config} PRIVATE HW_${hw_config})

   target_link_libraries(picoChippy_${hw_config} PRIVATE music VGM HW)

   set_target_properties(picoChippy_${hw_config} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

endforeach()
