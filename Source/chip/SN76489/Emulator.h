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

// \brief Emulation of Texas Instruments SN76489

#pragma once

#include "SN76489/Interface.h"

#undef  DBG
#define DBG if (0) printf

namespace SN76489 {

class Emulator : public SN76489::Interface
{
public:
   Emulator() = default;

   //! Set sample rate (Hz)
   void setClock(unsigned clock_) override
   {
      ticks_per_sample = 128 / CLOCK_DIV;

      SN76489::Interface::setClock(clock_);
   }

   //! Configure shift register size and taps
   void config(unsigned bits_, uint32_t taps_, uint8_t flags_) override
   {
      noise.config(bits_, taps_);
   }

   //! Add sample to the mix
   void mixOut(Sample& mix_)
   {
      int32_t sample{0};

      sample += tone[CH_TONE1].out(ticks_per_sample);
      sample += tone[CH_TONE2].out(ticks_per_sample);
      sample += tone[CH_TONE3].out(ticks_per_sample);
      sample += noise.out(ticks_per_sample);

      mixer(sample / 4, mix_);
   }

protected:
   //! Write a byte to the SN76489 bus
   virtual void writeBus(uint8_t data_) override
   {
      bool first_byte = (data_ & 0b10000000) != 0;
      if (first_byte)
      {
         // Latch register
         reg = (data_ >> 4) & 0b111;
      }

      unsigned channel  = reg >> 1;
      bool     is_atten = (reg & 0b001) != 0;

      if (is_atten)
      {
         uint8_t value = data_ & 0b1111;

         DBG("TONE%u ATTEN %2u dB\n", channel + 1, value * 2);

         if (channel == CH_NOISE)
            noise.setAtten(value);
         else
            tone[channel].setAtten(value);
      }
      else
      {
         if (channel == CH_NOISE)
         {
            DBG("NOISE FB=%u FREQ MODE=%u\n", (data_ >> 2) & 1, data_ & 0b11);
            noise_follows_tone3 = noise.control(data_);
         }
         else
         {
            if (first_byte)
               tone[channel].setPeriodLo(data_ & 0b1111);
            else
               tone[channel].setPeriodHi(data_ & 0b111111);

            DBG("TONE%u FREQ 0x%03x\n", channel + 1, tone[channel].getPeriod());
         }

         if ((channel >= CH_TONE3) && noise_follows_tone3)
         {
            noise.setPeriod(tone[CH_TONE3].getPeriod());
         }
      }
   }

   enum Channel
   {
      CH_TONE1 = 0,
      CH_TONE2,
      CH_TONE3,
      CH_NOISE,
      NUM_CHANNELS
   };

   //! Tone generator
   class Tone
   {
   public:
      Tone() = default;

      uint16_t getPeriod() const { return period; }

      //! Set 10-bit period
      void setPeriod(unsigned value_)
      {
         period = value_;
      }

      //! Set MS 4 bits of oscillator period
      void setPeriodHi(uint8_t value6_) { setPeriod((period & 0x00F) | (value6_ << 4)); }

      //! Set LS 6 bits of oscillator period
      void setPeriodLo(uint8_t value4_) { setPeriod((period & 0x3F0) | value4_); }

      //! Set 4-bit attenuation (x2 dB)
      void setAtten(uint8_t value4_)
      {
         // XXX assume full attenuation is a special case -inf dB not -30 dB
         static const int16_t atten_to_level[16] =
         {
            0x7FFF, 0x65AC, 0x50C3, 0x4026, 0x32F5, 0x287A, 0x2027, 0x198A,
            0x1449, 0x101D, 0x0CCD, 0x0A2B, 0x0813, 0x066A, 0x0518, 0x0000
         };

         level = level > 0 ? +atten_to_level[value4_]
                           : -atten_to_level[value4_];
      }

      //! Get output
      int16_t out(unsigned ticks_)
      {
         if (period == 0)
            return level;

         phase -= ticks_;

         while(phase <= 0)
         {
            phase += period;
            level = -level;
         }

         return level;
      }

   protected:
      int16_t  phase{};
      volatile uint16_t period{};
      volatile int16_t  level{};
   };

   //! Noise generator
   class Noise : public Tone
   {
   public:
      Noise() = default;

      //! Configure shift register size and taps
      void config(unsigned bits_, uint32_t taps_)
      {
         // LS bit of taps_ arg is final bit in shift-reg
         // reverse so that MS bit is final bit
         taps_ = ((taps_ & 0xAAAAAAAA) >>  1) | ((taps_ & 0x55555555) <<  1);
         taps_ = ((taps_ & 0xCCCCCCCC) >>  2) | ((taps_ & 0x33333333) <<  2);
         taps_ = ((taps_ & 0xF0F0F0F0) >>  4) | ((taps_ & 0x0F0F0F0F) <<  4);
         taps_ = ((taps_ & 0xFF00FF00) >>  8) | ((taps_ & 0x00FF00FF) <<  8);
         taps_ = ((taps_ & 0xFFFF0000) >> 16) | ((taps_ & 0x0000FFFF) << 16);

         width      = bits_;
         noise_mask = taps_ >> (32 - bits_);
      }

      //! Update control bits
      bool control(uint8_t value_)
      {
         shift_reg = 1;

         bool feedback = (value_ & 0b100) == 0b100;
         tap_mask      = feedback ? noise_mask
                                  : 1 << (width - 1);
         
         unsigned freq = value_ & 0b11;

         if (freq == 0b11)
         {
            return true;
         }
         else
         {
            setPeriod(1 << (freq + 4));
            return false;
         }
      }

      int16_t out(unsigned ticks_)
      {
         if (period == 0)
            return level;

         phase -= ticks_;

         while(phase <= 0)
         {
            phase += period;

            state = not state;

            if (state)
            {
               output = (shift_reg >> width) & 1;

               uint32_t taps = shift_reg & tap_mask;

               // effective XOR of the tapped bits
               taps ^= taps >> 16;
               taps ^= taps >> 8;
               taps ^= taps >> 4;
               taps ^= taps >> 2;
               taps ^= taps >> 1;

               shift_reg = (shift_reg << 1) | (taps & 1);
            }
         }

         return output ? level : -level;
      }

   private:
      bool state{};
      bool output{};

      volatile uint32_t noise_mask{0x9000};
      volatile uint32_t shift_reg{};
      volatile uint32_t tap_mask{0x9000};
      volatile uint8_t  width{16};
   };

   static const unsigned CLOCK_DIV = 16;

   unsigned ticks_per_sample{4};         //!< Number of clock ticks per sample
   unsigned reg{0};                      //!< Latched register number
   bool     noise_follows_tone3{false};
   Tone     tone[NUM_CHANNELS - 1];
   Noise    noise{};
};

}
