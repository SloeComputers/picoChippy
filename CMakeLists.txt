#-------------------------------------------------------------------------------
# Copyright (c) 2025 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)

project(picoChippy
        LANGUAGES CXX C ASM
        VERSION 0.02)

include(CTest)

set(HARDWARE picoX21H)

add_subdirectory(Platform/Hardware)
add_subdirectory(music)
add_subdirectory(Source)
