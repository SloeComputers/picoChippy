#-------------------------------------------------------------------------------
# Copyright (c) 2025 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

add_library(VGM INTERFACE)

target_include_directories(VGM
   INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
   )

target_link_libraries(VGM
   INTERFACE chip STB
   )

add_subdirectory(test)
