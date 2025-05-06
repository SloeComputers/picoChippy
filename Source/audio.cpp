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

#include "Audio.h"
#include "Synth.h"
#include "SynthIO.h"

#include "YM2151/Emulator.h"
#include "SN76489/Emulator.h"
#include "SegaPCM/Emulator.h"

#include "VGM/Decoder.h"

#if not defined(HW_NATIVE)

#include "MTL/MTL.h"
#include "MTL/Pins.h"

#include "MTL/chip/PioI2S_S16.h"

#include "YM2151/Hardware.h"

static YM2151::Hardware<MTL::Pio0,
                        MTL::Pio1,
                        /* CTRL4    */ MTL::PIN_4,
                        /* DATA8    */ MTL::PIN_14,
                        /* REV_DATA */ true> ym2151{};
static MTL::PioI2S_S16<MTL::Pio0> i2s_out{};

#else
static YM2151::Emulator ym2151{};
#endif

static SegaPCM::Emulator sega_pcm{};
static SN76489::Emulator sn76489{};
static Audio             audio{};
static VGM::Decoder      decoder{};

extern Synth        synth;


void SynthIO::triggerVGM()
{
   decoder.play();
}

void SynthIO::setVolume(uint8_t value_)
{
   audio.volume = value_;
}

void SynthIO::setBalance(uint8_t value_)
{
   audio.balance = value_;
}

#if not defined(HW_NATIVE)

static void runDAC()
{
   int32_t pcm_left, pcm_right;
   int16_t left, right;

   while(true)
   {
      decoder.tick();
      sega_pcm.getOut(pcm_left, pcm_right);

      // pcm_left = pcm_right = sn76489.getOut();

      ym2151.getOut(left, right);
      audio.process(left, right, pcm_left, pcm_right);
      i2s_out.push((left << 16) | (right & 0xFFFF));

      ym2151.getOut(left, right);
      audio.process(left, right, pcm_left, pcm_right);
      i2s_out.push((left << 16) | (right & 0xFFFF));
   }
}

#endif

void startAudio(const uint8_t* vgm_data_)
{
   decoder.load(vgm_data_);

#if not defined(HW_NATIVE)
   unsigned clock_hz = decoder.getClock();

   ym2151.init(clock_hz,
               /* CLK M       */ MTL::PIN_9,
               /* CLK SD SAM1 */ MTL::PIN_10);

   i2s_out.download(clock_hz, /* SD */ MTL::PIN_31, /* LRCLK SCLK */ MTL::PIN_32);
   i2s_out.start();
#endif

   synth.init(ym2151);

   decoder.plugSN76489(&sn76489);
   decoder.plugYM2151(&ym2151);
   decoder.plugSegaPCM(&sega_pcm);

#if not defined(HW_NATIVE)
   MTL_start_core(1, runDAC);
#endif
}
