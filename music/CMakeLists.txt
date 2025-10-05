#-------------------------------------------------------------------------------
# Copyright (c) 2025 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

autogen_py(Table_vgm ${CMAKE_CURRENT_SOURCE_DIR}/demo.vgm)

add_library(music STATIC
   Table_vgm.cpp)

target_include_directories(music
   PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
   )
