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

#include "YM2151/Interface.h"
#include "SegaPCM/Interface.h"

#undef  DBG
#define DBG if (0) printf

namespace VGM {

class Decoder
{
public:
   Decoder() = default;

   void load(const uint8_t* data_)
   {
      raw = data_;
      hdr = (const Header*)raw;
   }

   void dis()
   {
      printf("\n");
      printf("Size       : %u bytes\n", hdr->eof_offset + 4);
      printf("Version    : %x.%02x\n",  hdr->version >> 8, hdr->version & 0xFF);
      printf("YM2151 clk : %u Hz\n",    hdr->ym2151_clock);
      printf("VGM offset : +0x%x\n",    0x34 + hdr->vgm_data_offset);
      printf("Sega PCMclk: %u Hz\n",    hdr->sega_pcm_clock);
      printf("Sega PCMifc: 0x%08x\n",   hdr->sega_pcm_interface);
      printf("\n");
   }

   void play(YM2151::Interface*      ym2151_,
             SegaPCM::Interface<16>* sega_pcm_)
   {
      reset();

      while(true)
      {
         uint8_t byte = read8();
         switch(byte)
         {
         case 0x54:
            {
               uint8_t addr = read8();
               uint8_t data = read8();
   
               DBG("YM2151[0x%02x] = 0x%02x\n", data, addr);

               ym2151_->writeReg(addr, data);
            }
            break;

         case 0x61: wait(read16()); break;

         case 0x62: wait(735); break;

         case 0x63: wait(882); break;

         case 0x66: DBG("END\n"); return;

         case 0x67:
            {
               // assert(raw[offset++] = 0x66);
   
               skip(1);
               uint8_t  type = read8();
               uint32_t size = read32();

               DBG("DB type=0x%02x size=0x%x\n", type, size);

               skip(size);
            }
            break;

         case 0x70:
         case 0x71:
         case 0x72:
         case 0x73:
         case 0x74:
         case 0x75:
         case 0x76:
         case 0x77:
         case 0x78:
         case 0x79:
         case 0x7A:
         case 0x7B:
         case 0x7C:
         case 0x7D:
         case 0x7E:
         case 0x7F:
            wait((byte & 0xF) + 1);
            break;

         case 0xC0:
            {
               uint16_t addr = read16();
               uint8_t  data = read8();

               DBG("SEGA[0x%04x] = 0x%02x\n", addr, data);
            }
            break;

         default:
            DBG("ERROR %02X\n", byte);
            return;
         }
      }
   }

private:
   void reset()
   {
      offset = 0x34 + hdr->vgm_data_offset;
   }

   uint8_t read8() { return raw[offset++]; }

   uint16_t read16() { return read8() | (uint16_t(read8()) << 8); }

   uint32_t read32() { return read16() | (uint32_t(read16()) << 16); }

   void skip(unsigned bytes_) { offset += bytes_; }

   void wait(unsigned samples_)
   {
      DBG("Wait %u\n", samples_);
      usleep(samples_ * 23);
   }

   struct Header
   {
      char     ident[4];
      uint32_t eof_offset;
      uint32_t version;
      uint32_t sn76489_clock;

      uint32_t ym2413_clock;
      uint32_t gd3_offset;
      uint32_t total_samples;
      uint32_t loop_offset;

      uint32_t loop_samples;
      uint32_t rate;
      uint16_t sn76489_feedback;
      uint8_t  sn76489_shit_reg_width;
      uint8_t  sn76489_flags;
      uint32_t ym2612_clock;

      uint32_t ym2151_clock;
      uint32_t vgm_data_offset;
      uint32_t sega_pcm_clock;
      uint32_t sega_pcm_interface;
   };

   const Header*  hdr;
   const uint8_t* raw;
   unsigned       offset{0};
};

} // namespace VGM
