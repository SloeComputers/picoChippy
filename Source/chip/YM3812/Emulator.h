//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

// \brief Interface for Yamaha YM3812

#pragma once

#include "YM812/Interface.h"

namespace YM3812 {

class Emulator : public Interface
{
public:
   Emulator() = default;

   signed download(unsigned clock_freq_)
   {
      return 0;
   }

private:
   //! Write a byte to the YM2151 bus
   void writeBus(bool a0_, uint8_t value_) override
   {
   }

   //! 
   void waitForReady() override
   {
   }
};

} // namespace YM2151
