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

#include "Chip.h"

#undef  DBG
#define DBG if (0) printf

namespace VGM {

class Decoder
{
public:
   using TickFn = void (*)(void*);

   Decoder() = default;

   void plugSN76489( Chip* interface_) { sn76489   = interface_; }
   void plugSegaPCM( Chip* interface_) { sega_pcm  = interface_; }
   void plugOKIM6295(Chip* interface_) { oki_m6295 = interface_; }
   void plugYM2151(  Chip* interface_) { ym2151    = interface_; }

   //! Load VGM image
   void load(const uint8_t* image_)
   {
      stop();

      raw = image_;
      hdr = (const Header*)raw;

      hdr->dis();

      configSynths();
   }

   void play()
   {
      reset();

      playing = true;
   }

   void tick()
   {
      if (not playing) return;

      uint16_t addr16;
      uint8_t  addr;
      uint8_t  data;

      uint8_t byte = read8();

      switch(byte)
      {
      case 0x50:
         data = read8();
         if (sn76489) sn76489->write(0, data);
         break;

      case 0x54:
         addr = read8();
         data = read8();
         if (ym2151) ym2151->write(addr, data);
         break;

      case 0x5A:
         addr = read8();
         data = read8();
         // if (ym3812) ym2812->write(addr, data);
         break;

      case 0x61: wait(read16()); break;
      case 0x62: wait(735); break;
      case 0x63: wait(882); break;

      case 0x66: DBG("END\n"); playing = false; break;

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

               case 0x8B:
                  DBG("OKI M6295 ROM %04x/%04x +%04X %p\n", address, rom_size, size, ptr8());
                  oki_m6295->addSample(address, ptr8(), size);
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

      case 0xB8:
         addr = read8();
         data = read8();
         if (oki_m6295) oki_m6295->write(0, data); break;
         break;

      case 0xC0:
         addr16 = read16();
         data   = read8();
         if (sega_pcm) sega_pcm->write(addr16, data);
         break;

      default:
         DBG("ERROR %02X\n", byte);
         break;
      }
   }

   void stop()
   {
      playing =  false;
   }

   void sample()
   {
      samples++;
   }

private:
   //! Configure synths for current VGM
   void configSynths()
   {
      if (sn76489 && sn76489->setClock(hdr->getSN76489Clock()))
      {
         if (hdr->version >= 110)
         {
            sn76489->config(hdr->sn76489_shift_reg_width,
                            hdr->sn76489_feedback,
                            hdr->sn76489_flags);
         }
      }

      if (sega_pcm) sega_pcm->setClock(hdr->getSegaPCMClock());

      if (oki_m6295) oki_m6295->setClock(hdr->getOKIM6295Clock());

      if (ym2151) ym2151->setClock(hdr->getYM2151Clock());
   }

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

   bool playing{false};

   Chip* sn76489{};
   Chip* ym2151{};
   Chip* sega_pcm{};
   Chip* oki_m6295{};
};

} // namespace VGM
