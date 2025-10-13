//-------------------------------------------------------------------------------
// Copyright (c) 2023 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdint>

//! I/O interfaces for the Synth
class SynthIO
{
public:
   SynthIO() = default;

   //! Display on text on 16x2 LCD
   void displayLCD(unsigned row_, const char* text);

   //! Start/stop VGM player
   void playVGM(bool start_);
};
