#-------------------------------------------------------------------------------
# Copyright (c) 2025 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)

project(picoChippy
        LANGUAGES CXX C ASM
        VERSION 0.02)

include(CTest)

set(HWR_CONFIG picoX21H)

add_subdirectory(Platform)
add_subdirectory(music)
add_subdirectory(Source)
