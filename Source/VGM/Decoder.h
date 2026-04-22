//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <cstdio>
#include <unistd.h>

#if !defined(MTL_TARGET)
#include <atomic>
#endif

#include "VGM/Header.h"

#include "Chip.h"
#include "Dac.h"

#undef  DBG
#define DBG if (0) printf

namespace VGM {

class Decoder
{
public:
   using TickFn = void (*)(void*);

   Decoder() = default;

   void plugDAC(     chip::Dac* dac_)  { dac       = dac_; }

   void plugSN76489( Chip* interface_) { sn76489   = interface_; }

   void plugYM2413(  Chip* interface_) { ym2413    = interface_; }
   void plugYM2612(  Chip* interface_) { ym2612    = interface_; }
   void plugYM2151(  Chip* interface_) { ym2151    = interface_; }
   void plugYM2608(  Chip* interface_) { ym2608    = interface_; }
   void plugYM2610(  Chip* interface_) { ym2610    = interface_; }
   void plugYM3812(  Chip* interface_) { ym3812    = interface_; }
   void plugYM3526(  Chip* interface_) { ym3526    = interface_; }
   void plugY8950(   Chip* interface_) { y8950     = interface_; }
   void plugYMZ280B( Chip* interface_) { ymz280b   = interface_; }
   void plugYMF262(  Chip* interface_) { ymf262    = interface_; }

   void plugSegaPCM( Chip* interface_) { sega_pcm  = interface_; }
   void plugOKIM6295(Chip* interface_) { oki_m6295 = interface_; }

   //! Load VGM image
   void load(const uint8_t* image_)
   {
      raw = image_;

      mode = SKIP;
   }

   void play()
   {
      mode = PLAY;
   }

   void stop()
   {
      mode = STOP;
   }

   void tick()
   {
      switch(mode)
      {
      case STOPPED:
         return;

      case PLAY:
         reset();
         mode = PLAYING;
         break;

      case PLAYING:
         break;

      case STOP:
         reset();
         mode = STOPPED;
         return;

      case SKIP:
         mode = PLAY;
         return;
      }

      uint16_t addr;
      uint8_t  data;

      uint8_t byte = read8();

      switch(byte)
      {
      case 0x50:
         data = read8();
         if (sn76489) sn76489->write(0, data);
         break;

      case 0x51:
         addr = read8();
         data = read8();
         if (ym2413) ym2413->write(addr, data);
         break;

      case 0x52:
         addr = read8();
         data = read8();
         if (ym2612) ym2612->write(addr, data);
         break;

      case 0x53:
         addr = read8();
         data = read8();
         if (ym2612) ym2612->write(0x100 + addr, data);
         break;

      case 0x54:
         addr = read8();
         data = read8();
         if (ym2151) ym2151->write(addr, data);
         break;

      case 0x55:
         addr = read8();
         data = read8();
         if (ym2203) ym2203->write(addr, data);
         break;

      case 0x56:
         addr = read8();
         data = read8();
         if (ym2608) ym2608->write(addr, data);
         break;

      case 0x57:
         addr = read8();
         data = read8();
         if (ym2608) ym2608->write(0x100 + addr, data);
         break;

      case 0x58:
         addr = read8();
         data = read8();
         if (ym2610) ym2610->write(addr, data);
         break;

      case 0x59:
         addr = read8();
         data = read8();
         if (ym2610) ym2610->write(0x100 + addr, data);
         break;

      case 0x5A:
         addr = read8();
         data = read8();
         if (ym3812) ym3812->write(addr, data);
         break;

      case 0x5B:
         addr = read8();
         data = read8();
         if (ym3526) ym3526->write(addr, data);
         break;

      case 0x5C:
         addr = read8();
         data = read8();
         if (y8950) y8950->write(addr, data);
         break;

      case 0x5D:
         addr = read8();
         data = read8();
         if (ymz280b) ymz280b->write(addr, data);
         break;

      case 0x5E:
         addr = read8();
         data = read8();
         if (ymf262) ymf262->write(addr, data);
         break;

      case 0x5F:
         addr = read8();
         data = read8();
         if (ymf262) ymf262->write(0x100 + addr, data);
         break;

      case 0x61: wait(read16()); break;
      case 0x62: wait(735); break;
      case 0x63: wait(882); break;

      case 0x66: DBG("END\n"); stop(); break;

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
         if (oki_m6295) oki_m6295->write(0, data);
         break;

      case 0xC0:
         addr = read16();
         data = read8();
         if (sega_pcm) sega_pcm->write(addr, data);
         break;

      default:
         printf("ERROR %02X\n", byte);
         stop();
         break;
      }
   }

   void sample()
   {
      samples++;
   }

private:
   //! Configure synths for current VGM
   void configSynths()
   {
      hdr = (const Header*)raw;
      hdr->dis();

      unsigned sample_freq_hz = 0;

      if (sn76489 && sn76489->setClock(hdr->getSN76489Clock()))
      {
         if (hdr->version >= 110)
         {
           sn76489->config(hdr->sn76489_shift_reg_width,
                           hdr->sn76489_feedback,
                           hdr->sn76489_flags);
         }

         sample_freq_hz = sn76489->getSampleFreq();
      }

      if (sega_pcm && sega_pcm->setClock(hdr->getSegaPCMClock()))
      {
         sample_freq_hz = sega_pcm->getSampleFreq();
      }

      if (oki_m6295 && oki_m6295->setClock(hdr->getOKIM6295Clock()))
      {
         sample_freq_hz = oki_m6295->getSampleFreq();
      }

      if (ym2151 && ym2151->setClock(hdr->getYM2151Clock()))
      {
         sample_freq_hz = ym2151->getSampleFreq();
      }

      printf("sample freq = %u Hz\n", sample_freq_hz);

      dac->setSampleRate(sample_freq_hz);
   }

   void reset()
   {
      configSynths();

      offset = 0x34 + hdr->vgm_data_offset;

      if (sn76489)   sn76489->reset();

      if (ym2413)    ym2413->reset();
      if (ym2612)    ym2612->reset();
      if (ym2151)    ym2151->reset();
      if (ym2203)    ym2203->reset();
      if (ym2608)    ym2608->reset();
      if (ym2610)    ym2610->reset();
      if (ym3812)    ym3812->reset();
      if (ym3526)    ym3526->reset();
      if (y8950)     y8950->reset();
      if (ymz280b)   ymz280b->reset();
      if (ymf262)    ymf262->reset();

      if (sega_pcm)  sega_pcm->reset();
      if (oki_m6295) oki_m6295->reset();
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

   enum Mode { STOPPED, PLAY, PLAYING, SKIP, STOP };

   Mode mode{STOPPED};

   chip::Dac* dac{};

   Chip* sn76489{};
   Chip* ym2413{};
   Chip* ym2612{};
   Chip* ym2151{};
   Chip* ym2203{};
   Chip* ym2608{};
   Chip* ym2610{};
   Chip* ym3812{};
   Chip* ym3526{};
   Chip* y8950{};
   Chip* ymz280b{};
   Chip* ymf262{};

   Chip* sega_pcm{};
   Chip* oki_m6295{};
};

} // namespace VGM
