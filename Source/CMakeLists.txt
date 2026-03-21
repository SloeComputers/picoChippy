#-------------------------------------------------------------------------------
# Copyright (c) 2025 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

add_subdirectory(chip)
add_subdirectory(VGM)

add_picoX21H_executable(picoChippy
                          picoChippy.cpp
                       LIBRARIES
                          music VGM)
