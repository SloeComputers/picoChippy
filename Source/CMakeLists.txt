#-------------------------------------------------------------------------------
# Copyright (c) 2025 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

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
