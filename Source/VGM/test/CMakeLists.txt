#-------------------------------------------------------------------------------
# Copyright (c) 2025 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

if(${PLT_NATIVE})

   add_executable(test_VGM
      testMain.cpp
      testVGMDecoder.cpp
      )

   target_link_libraries(test_VGM
      PRIVATE music VGM)

   add_test(NAME test_VGM COMMAND test_VGM)

endif()
