//------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------

// \brief Digital to Analogue Convectior

#pragma once

#include "hw/Config.h"

#if defined(HW_DAC_I2S)
#include "MTL/chip/PioI2S_S16.h"

#elif defined(HW_DAC_PWM)
#include "MTL/chip/PwmAudio.h"

#elif defined(HW_DAC_NATIVE)
#include "PLT/Audio.h"

#else
#error "DAC config not set"
#endif

namespace hw {

#if defined(HW_DAC_I2S)

class Dac
{
public:
   Dac()
   {
      MTL::config.gpio(HW_DAC_I2S_SD,       "I2S SD");
      MTL::config.gpio(HW_DAC_I2S_CLKS,     "I2S SCLK");
      MTL::config.gpio(HW_DAC_I2S_CLKS + 1, "I2S LRCLK");
   }

   void start(unsigned sample_rate_)
   {
      i2s.download(sample_rate_ * BITS_PER_FRAME * 2,
                   HW_DAC_I2S_SD, HW_DAC_I2S_CLKS);
      i2s.start();
   }

   void push(uint32_t sample_pair_)
   {
      i2s.push(sample_pair_);
   }

private:
   static const unsigned BITS_PER_FRAME = 32;

   MTL::PioI2S_S16<MTL::Pio0> i2s{};
};

#elif defined(HW_DAC_PWM)

class Dac
{
public:
   Dac() = default;

   void start(unsigned sample_rate_)
   {
   }

   void push(uint32_t sample_pair_)
   {
   }

private:
};

#elif defined(HW_DAC_NATIVE)

class Dac
{
public:
   Dac() = default;

   void start(unsigned sample_rate_)
   {
   }

   void push(uint32_t sample_pair_)
   {
   }

private:
};

#endif

} // namespace hw
