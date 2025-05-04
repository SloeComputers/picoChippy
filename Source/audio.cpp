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

// \brief Audio processing

#if not defined(HW_NATIVE)

#include "VGM/Player.h"

#if defined(HW_NATIVE)
#include "YM2151/Emulator.h"
#else
#include "YM2151/Hardware.h"
#endif

#include "SegaPCM/Emulator.h"

#include "MTL/MTL.h"
#include "MTL/Pins.h"

#include "MTL/chip/PioYMDAC.h"
#include "MTL/chip/PioI2S_S16.h"

#if defined(HW_NATIVE)
static YM2151::Emulator ym2151{};
#else
static YM2151::Hardware<MTL::Pio0,
                        MTL::Pio1,
                        /* CTRL4    */ MTL::PIN_4,
                        /* DATA8    */ MTL::PIN_14,
                        /* REV_DATA */ true> ym2151{};
#endif

static SegaPCM::Emulator sega_pcm{};

static MTL::PioI2S_S16<MTL::Pio0> i2s_out{};

extern VGM::Player vgm_player;

static void runDAC()
{
   int32_t pcm_left, pcm_right;
   int16_t left, right;

   while(true)
   {
      vgm_player.tick();
      sega_pcm.getOut(pcm_left, pcm_right);

      ym2151.dac_in.pop(left, right);
      vgm_player.audio.process(left, right, pcm_left, pcm_right);
      i2s_out.push((left << 16) | (right & 0xFFFF));

      ym2151.dac_in.pop(left, right);
      vgm_player.audio.process(left, right, pcm_left, pcm_right);
      i2s_out.push((left << 16) | (right & 0xFFFF));
   }
}

void startAudio()
{
   static const unsigned YM2151_CLOCK_HZ = 4000000; //!< 4 MHz

   ym2151.init(YM2151_CLOCK_HZ,
               /* CLK M       */ MTL::PIN_9,
               /* CLK SD SAM1 */ MTL::PIN_10);

   i2s_out.download(YM2151_CLOCK_HZ, /* SD */ MTL::PIN_31, /* LRCLK SCLK */ MTL::PIN_32);
   i2s_out.start();

   vgm_player.init(ym2151, sega_pcm);

   MTL_start_core(1, runDAC);
}

#endif
