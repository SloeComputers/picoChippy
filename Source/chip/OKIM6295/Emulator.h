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

// \brief Emulation of OKIM6295 "4-channel mixing ADPCM voice synthesis LSI"

#pragma once

#include <cstring>

#include "OKIM6295/Interface.h"

#undef  DBG
#define DBG if (0) printf

namespace OKIM6295 {

class Emulator : public Interface
{
public:
   Emulator() = default;

   //! Get next PCM sample pair
   void mixOut(Sample& mix_)
   {
      if (mute) return;

      Sample out{};

      for(unsigned i = 0; i < NUM_CHANNELS; ++i)
      {
         channel[i].mixOut(out);
      }

      mixer(out, mix_);
   }

private:
   //! Write a byte to the OKIM6295 bus
   virtual void writeBus(uint8_t data_) override
   {
      DBG("OKIM %02X\n", data_);

      if (chan_atten_byte)
      {
         chan_atten_byte = false;

         unsigned       atten = data_ & 0xF;
         const uint8_t* start = getStartPtr(sample_id);
         const uint8_t* end   = getEndPtr(sample_id);

         switch(data_ >> 4)
         {
         case 0b0001: channel[0].on(start, end, atten); break;
         case 0b0010: channel[1].on(start, end, atten); break;
         case 0b0100: channel[2].on(start, end, atten); break;
         case 0b1000: channel[3].on(start, end, atten); break;
         }
      }
      else if (data_ & (1<<7))
      {
         sample_id       = data_ & 0x7F;
         chan_atten_byte = true;
      }
      else
      {
         if (data_ & 0b0001000) channel[0].off();
         if (data_ & 0b0010000) channel[1].off();
         if (data_ & 0b0100000) channel[2].off();
         if (data_ & 0b1000000) channel[3].off();
      }
   }

   class Channel
   {
   public:
      Channel() = default;

      void on(const uint8_t* start_, const uint8_t* end_, unsigned atten_)
      {
         memory = nullptr;

         adpcmReset();

         vol    = 1;
         end    = end_;
         memory = start_;
      }

      void off()
      {
         memory = nullptr;
      }

      //! Get next PCM sample pair
      void mixOut(Sample& mix_)
      {
         if (memory == nullptr)
            return;

         if (memory == end)
         {
            memory = nullptr;
            printf("");
            return;
         }

         uint8_t input;

         if (read_sample_pair)
         {
            sample_pair = *memory++;

            input            = sample_pair >> 4;
            read_sample_pair = false;
         }
         else
         {
            input            = sample_pair & 0xF;
            read_sample_pair = true;
         }

         int16_t sample = adpcmDecode(input) << 4;

         mix_.left  += sample * vol;
         mix_.right += sample * vol;
      }

   private:
      void adpcmReset()
      {
         adpcm_step_index = 0;
         adpcm_step_size  = 16;
         adpcm_prev_out   = 0;
      }

      //! Decode a 5-but ADPCM sample to a 12-bit linear sample
      int32_t adpcmDecode(uint8_t in4)
      {
         static const signed step_adjust_table[8] =
         {
            -1, -1, -1, -1, +2, +4, +6, +8
         };

         static const uint16_t step_size_table[49] =
         {
             16,  17,  19,  21,  23,  25,  28,  31,   34,   37,   41,  45,
             50,  55,  60,  66,  73,  80,  88,  97,  107,  118,  130, 143,
            157, 173, 190, 209, 230, 253, 279, 307,  337,  371,  408, 449,
            494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411,
            1552
         };

         // Compute difference from previous step size
         int32_t diff = ((in4 & 0b111) * adpcm_step_size) >> 3;
         if (in4 & 0b1000)
            diff = -diff;

         // Compute next step size
         adpcm_step_index += step_adjust_table[in4 & 0b111];
         if (adpcm_step_index < 0)
            adpcm_step_index = 0;
         else if (adpcm_step_index > 48)
            adpcm_step_index = 48;
         adpcm_step_size = step_size_table[adpcm_step_index];

         int32_t out12  = diff + adpcm_prev_out;
         adpcm_prev_out = out12;
         return out12;
      }

      signed   adpcm_step_index{0};
      unsigned adpcm_step_size{16};
      unsigned adpcm_prev_out{0};

      bool           read_sample_pair{true};
      uint8_t        vol{};
      const uint8_t* end{};
      const uint8_t* memory{}; //!< XXX volatile to enable hacky synchronization
      uint8_t        sample_pair;
   };

   Channel  channel[NUM_CHANNELS];
   unsigned sample_id{};
   bool     chan_atten_byte{false};
};

} // namespace OKIM6295
