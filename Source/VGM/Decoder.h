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

#pragma once

#include <cstdint>
#include <cstdio>
#include <unistd.h>

#if !defined(MTL_TARGET)
#include <atomic>
#endif

#include "VGM/Header.h"

#include "SN76489/Interface.h"
#include "YM2151/Interface.h"
#include "SegaPCM/Interface.h"

#undef  DBG
#define DBG if (0) printf

namespace VGM {

class Decoder
{
public:
   Decoder() = default;

   unsigned getClock() const
   {
      if (hdr->sn76489_clock != 0)
         return hdr->sn76489_clock;

      if (hdr->ym2151_clock != 0)
         return hdr->ym2151_clock;

      if (hdr->sega_pcm_clock != 0)
         return hdr->sega_pcm_clock;

      return 4000000;
   }

   //! Load VGM image
   void load(const uint8_t* image_)
   {
      raw = image_;
      hdr = (const Header*)raw;

      hdr->dis();
   }

   void plugSN76489(SN76489::Interface* interface_)
   {
      sn76489 = hdr->sn76489_clock != 0 ? interface_
                                        : nullptr;

      if (sn76489)
      {
         sn76489->setClock(hdr->sn76489_clock);

         sn76489->config(hdr->sn76489_shift_reg_width,
                         hdr->sn76489_feedback,
                         hdr->sn76489_flags);
      }
   }

   void plugYM2151(YM2151::Interface* interface_)
   {
      ym2151 = hdr->ym2151_clock != 0 ? interface_
                                      : nullptr;
   }

   void plugSegaPCM(SegaPCM::Interface* interface_)
   {
      sega_pcm = hdr->sega_pcm_clock != 0 ? interface_
                                          : nullptr;
   }

   void play()
   {
      reset();

      while(true)
      {
         uint8_t  byte = read8();

         switch(byte)
         {
         case 0x50: if (sn76489) sn76489->writeReg(read8()); break;

         case 0x54: if (ym2151) ym2151->writeReg(read8(), read8()); break;
         // case 0x5A: if (ym3812_) ym3812_->writeReg(read8(), read8()); break;

         case 0x61: wait(read16()); break;
         case 0x62: wait(735); break;
         case 0x63: wait(882); break;

         case 0x66: DBG("END\n"); return;

         case 0x67:
            {
               // assert(raw[offset] = 0x66);
   
               skip(1);
               uint8_t  type = read8();
               uint32_t size = read32();

               DBG("DB type=0x%02x size=0x%x\n", type, size);
 
               if ((type >= 0x80) && (type < 0xC0))
               {
                  uint32_t rom_size = read32();
                  uint32_t address  = read32();

                  size -= 8;

                  switch(type)
                  {
                  case 0x80:
                     DBG("SEGA ROM %04x/%04x +%04X %p\n", address, rom_size, size, ptr8());
                     sega_pcm->addSample(address, ptr8(), size);
                     break;

                  default:
                     DBG("???? ROM %04x/%04x +%04X\n", address, rom_size, size);
                     break;
                  }
               }

               skip(size);
            }
            break;

         case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
         case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
            wait((byte & 0xF) + 1);
            break;

         case 0xC0: if (sega_pcm) sega_pcm->writeReg(read16(), read8()); break;

         default:
            DBG("ERROR %02X\n", byte);
            return;
         }
      }
   }

   void tick()
   {
      samples++;
   }

private:
   void reset()
   {
      offset = 0x34 + hdr->vgm_data_offset;
   }

   const uint8_t* ptr8() { return &raw[offset]; }

   uint8_t read8() { return raw[offset++]; }

   uint16_t read16() { return read8() | (uint16_t(read8()) << 8); }

   uint32_t read32() { return read16() | (uint32_t(read16()) << 16); }

   void skip(unsigned bytes_) { offset += bytes_; }

   void wait(unsigned samples_)
   {
      DBG("Wait %u\n", samples_);
      usleep(samples_ * 23);
   }

   const Header*  hdr;
   const uint8_t* raw;
   unsigned       offset{0};

#if defined(MTL_TARGET)
   // TODO check/fix std::atomic support in MTL
   volatile unsigned samples{0};
#else
   std::atomic<unsigned> samples{0};
#endif

   SN76489::Interface* sn76489{};
   YM2151::Interface*  ym2151{};
   SegaPCM::Interface* sega_pcm{};
};

} // namespace VGM
