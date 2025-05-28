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

namespace VGM {

struct Header
{
   Header() = default;

   unsigned size() const { return eof_offset + 4; }

   void dis() const
   {
      printf("\n");
      printf("VGM v%x.%02x\n", version >> 8, version & 0xFF);
      printf("---------\n");
      printf("Size         : 0x%X (%u bytes)\n", size(), size());
      printf("GD3  offset  : +0x%x\n",    0x14 + gd3_offset);
      printf("Data offset  : +0x%x\n",    0x34 + vgm_data_offset);
      printf("Loop offset  : +0x%x\n",    0x1C + loop_offset);
      printf("Total samples: %u\n",       total_samples);
      printf("Loop  samples: %u\n",       loop_samples);
      printf("Rate         : %u\n",       rate);
      printf("\n");

      if (sn76489_clock)
      {
         printf("SN76489 clk  : %u Hz\n",   sn76489_clock);
         printf("SN76489 SR sz: %u bits\n", sn76489_shift_reg_width);
         printf("SN76489 fb   : 0x%04X\n",  sn76489_feedback);
         printf("SN76489 flag : 0x%02X\n",  sn76489_flags);
      }

      if (ym2151_clock)
         printf("YM2151 clk   : %u Hz\n", ym2151_clock);

      if (sega_pcm_clock)
      {
         printf("SegaPCM clk  : %u Hz\n", sega_pcm_clock);
         printf("SegaPCM ifc  : 0x%08X\n", sega_pcm_interface);
      }

      if (ym3812_clock)
         printf("YM3812 clk   : %u Hz\n", ym3812_clock);

      if (oki_m6295_clock)
         printf("OKIM6295 clk : %u Hz\n", oki_m6295_clock);

      printf("\n");
   }

   static const unsigned VERSION_MAJOR = 0x1;  // XXX BCD
   static const unsigned VERSION_MINOR = 0x51; // XXX BCD

   char     ident[4] = {'V', 'g', 'm', ' '};
   uint32_t eof_offset{};
   uint32_t version{(VERSION_MAJOR << 8) | VERSION_MINOR};
   uint32_t sn76489_clock{};

   uint32_t ym2413_clock{};
   uint32_t gd3_offset{};
   uint32_t total_samples{};
   uint32_t loop_offset{};

   uint32_t loop_samples{};
   uint32_t rate{};
   uint16_t sn76489_feedback{};
   uint8_t  sn76489_shift_reg_width{};
   uint8_t  sn76489_flags{};
   uint32_t ym2612_clock{};

   uint32_t ym2151_clock{};
   uint32_t vgm_data_offset{};
   uint32_t sega_pcm_clock{};
   uint32_t sega_pcm_interface{};

   uint32_t rf5c68_clock{};
   uint32_t ym2203_clock{};
   uint32_t ym2608_clock{};
   uint32_t ym2610_B_clock{};

   uint32_t ym3812_clock{};
   uint32_t ym3526_clock{};
   uint32_t y8950_clock{};
   uint32_t ymf262_clock{};

   uint32_t ymf278b_clock{};
   uint32_t ymf271_clock{};
   uint32_t ymz280b_clock{};
   uint32_t rf5c164_clock{};

   uint32_t pwm_clock{};
   uint32_t ay8910_clock{};
   uint32_t ayt_ay_flags{};
   uint32_t vm___lb_lm{};

   uint32_t gb_dmg_clock{};
   uint32_t nes_apu_clock{};
   uint32_t multipcm_clock{};
   uint32_t upd7759_clock{};

   uint32_t oki_m6258_clock{};
   uint32_t of_kf_cf{};
   uint32_t oki_m6295_clock{};
   uint32_t k051649_clock{};
};

} // namespace VGM
