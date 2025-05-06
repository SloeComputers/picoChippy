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

// \brief Interface for Texas Instruments SN76489

#pragma once

#include <cstdint>

#undef  DBG
#define DBG if (0) printf

namespace SN76489 {

class Interface
{
public:
   Interface() = default;

   //! Set clock frequency (Hz)
   virtual void setClock(unsigned clock_) = 0;

   //! Configure shift register size and taps
   virtual void config(unsigned bits_, uint32_t taps_, uint8_t flags_) {}

   //! Initialize all channels
   void reset()
   {
      for(unsigned ch = 0; ch < 4; ++ch)
      {
         noteOff(ch);
      }
   }

   //! Set channel (0-2) frequency
   void setFreq(unsigned channel_, unsigned freq10_)
   {
      unsigned freq_ms4 = (freq10_ >> 6) & 0b1111;
      unsigned freq_ls6 = freq10_ & 0b111111;
      unsigned reg      = channel_ * 2;

      writeReg(0b10000000 | (reg << 4) | freq_ms4);
      writeReg(freq_ls6);
   }

   //! Set channel 3 to noise
   void selectNoise(unsigned rate2_)
   {
      writeReg(0b11100100 | (rate2_ & 0b11));
   }

   //! Set channel 3 to periodic
   void selectPeriodic(unsigned rate2_)
   {
      writeReg(0b11100000 | (rate2_ & 0b11));
   }

   //! Set channel (0-3) attenuation 
   void setAtten(unsigned channel_, unsigned atten4_)
   {
      unsigned reg = 0b001 | (channel_ * 2);

      writeReg(0b10000000 | (reg << 4) | (atten4_ & 0b1111));
   }

   //! Play a note
   void noteOn(unsigned channel_, unsigned atten4_, unsigned freq10_)
   {
      setFreq(channel_, freq10_);
      setAtten(channel_, atten4_);
   }

   //! Stop a note
   void noteOff(unsigned channel_)
   {
      setAtten(channel_, 0xF);
   }

   void writeReg(uint8_t data_)
   {
      DBG("WR %02X\n", data_);

      writeBus(data_);
   }

protected:
   //! Write a byte to the SN76489 bus
   virtual void writeBus(uint8_t value_) = 0;
};

}
