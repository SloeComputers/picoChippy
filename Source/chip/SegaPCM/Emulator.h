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

// \brief Emulation of SegaPCM 315-3218

#pragma once

#include <cstring>

#include "SegaPCM/Interface.h"

#undef  DBG
#define DBG if (0) printf

namespace SegaPCM {

class Emulator : public Interface
{
public:
   Emulator() = default;

   //! Get next PCM sample pair
   void mixOut(Sample& mix_)
   {
      Sample out{};
      for(unsigned i = 0; i < NUM_CHANNELS; ++i)
      {
         chan[i].mixOut(out);
      }

      mixer(out, mix_);
   }

private:
   //! Write a byte to the SegaPCM bus
   virtual void writeBus(uint16_t addr_, uint8_t data_) override
   {
      unsigned n   = (addr_ & 0b01111000) >> 3;
      unsigned reg =  addr_ & 0b10000111;

      DBG("SEGA[0x%04X] : ch%02u.reg[0x%02x] = 0x%02X\n", addr_, n, reg, data_);

      chan[n].writeReg(reg, data_, this);
   }

   class Channel
   {
   public:
      Channel() = default;

      void writeReg(uint8_t addr_, uint8_t data_, Emulator* emulator_)
      {
         uint8_t ram_bank;

         switch(addr_)
         {
         case 0x00: /* ? */ break;
         case 0x01: /* ? */ break;

         case 0x02: lft_vol = data_ & 0x7F; break;
         case 0x03: rgt_vol = data_ & 0x7F; break;

         case 0x04: memory = nullptr; loop_addr = (loop_addr & 0xFF00) | data_; break;
         case 0x05: memory = nullptr; loop_addr = (loop_addr & 0x00FF) | (data_ << 8); break;
         case 0x06: memory = nullptr; end_block = data_ + 1; break;
         case 0x07: freq_8 = data_; break;

         case 0x80: /* ? */ break;
         case 0x81: /* ? */ break;
         case 0x82: /* ? */ break;
         case 0x83: /* ? */ break;

         case 0x84: memory = nullptr; accum_8 = (accum_8 & 0xFFF0000) | (data_ <<  8); break;
         case 0x85: memory = nullptr; accum_8 = (accum_8 & 0xF00FF00) | (data_ << 16); break;

         case 0x86:
            memory = nullptr;

            if ((data_ & (1<<0)) == 0)
            {
               // XXX this is the only register write that can enable the PCM channel
               ram_bank = (data_ & 0b01110000) >> 4;
               loop     = (data_ & (1<<1)) == 0;
               accum_8  = (accum_8 & 0x0FFFF00) | (ram_bank << 24);
               memory   = emulator_->getSamplePtr(accum_8 >> 8);
            }
            break;

         case 0x87: /* ? */ break;

         default:
            break;
         }
      }

      //! Get next PCM sample pair
      void mixOut(Sample& mix_)
      {
         if (memory == nullptr)
            return;

         // XXX there is a small hazard here - should probbaly only call
         //     writeReg() on the audio thread to avoid

         int32_t value = memory[accum_8 >> 8] - 0x80;

         mix_.left  += value * lft_vol;
         mix_.right += value * rgt_vol;

         accum_8 += freq_8;

         if (((accum_8 >> 16) & 0xFF) >= end_block)
         {
            if (loop)
               accum_8 = loop_addr << 8;
            else
               memory = nullptr;
         }
      }

   private:
      uint32_t accum_8{0};
      uint32_t freq_8{0};
      uint16_t loop_addr{0};
      uint8_t  end_block{0};
      uint8_t  rgt_vol{0};
      uint8_t  lft_vol{0};
      bool     loop{false};

      volatile const uint8_t* memory{}; //!< XXX volatile to enable hacky synchronization
   };

   Channel chan[NUM_CHANNELS];
};

} // namespace SegaPCM
