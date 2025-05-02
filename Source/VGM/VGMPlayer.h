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

#include <cstdio>
#include <unistd.h>

#include "STB/MIDIInstrument.h"

#include "VGMAudio.h"
#include "VGMDecoder.h"

#if defined(HW_NATIVE)
#include "YM2151/Emulator.h"
#else
#include "YM2151/Hardware.h"
#endif
#include "SegaPCM/Emulator.h"

#include "../SynthIO.h"

#include "Table_vgm.h"

namespace VGM {

class Player : public MIDI::Instrument</* N */ 8>
{
public:
   Player(SynthIO& synth_io_)
      : io(synth_io_)
   {
   }

   unsigned start(unsigned ym2151_clock_hz_ = YM2151_CLOCK_HZ)
   {
      ym2151.download(ym2151_clock_hz_);
      ym2151.start();

      io.displayLCD(0, " Cambridge pico ");
      io.displayLCD(1, " -*- Chippy -*- ");

      usleep(1000000);

      decoder.load(table_vgm);
      decoder.dis();

      for(unsigned i = 0; i < NUM_VOICES; ++i)
      {
         voiceInit(i);
      }

      //unsigned sample_rate = ym2151_clock_hz_ / (/* divider */ 2 * /* bits */ 16 * /* chans */ 2);

      return ym2151_clock_hz_;
   }

   Audio             audio{};
   SegaPCM::Emulator sega_pcm{};

private:
   void voiceInit(unsigned index_)
   {
      // Config channel ops
      for(unsigned i = 0; i < /* NUM_OP */ 1; ++i)
      {
         uint8_t op;
         switch(i)
         {
         case 0: op = YM2151::OP_M1; break;
         case 1: op = YM2151::OP_M2; break;
         case 2: op = YM2151::OP_C1; break;
         case 3: op = YM2151::OP_C2; break;
         }

         ym2151.setOp<YM2151::EG_AR>( index_, op, 31);
         ym2151.setOp<YM2151::EG_D1R>(index_, op, 0);
         ym2151.setOp<YM2151::EG_D1L>(index_, op, 0);
         ym2151.setOp<YM2151::EG_D2R>(index_, op, 0);
         ym2151.setOp<YM2151::EG_RR>( index_, op, 15);

         ym2151.setOp<YM2151::EG_TL>( index_, op, 10);

         ym2151.setOp<YM2151::MUL>(index_, op, 1);
      }

      // Config channel
      ym2151.setCh<YM2151::CONECT>(index_, 7);
      ym2151.setCh<YM2151::FB>(    index_, 0);
      ym2151.setCh<YM2151::RL>(    index_, 0b11);

      ym2151.setCh<YM2151::KF>(    index_, 0);
      ym2151.setCh<YM2151::AMS>(   index_, 0);
      ym2151.setCh<YM2151::PMS>(   index_, 0);
   }

   void voiceMute(unsigned index_) override
   {
   }

   void voiceOn(unsigned index_, uint8_t midi_note_, uint8_t velocity_) override
   {
      decoder.play(&ym2151, &sega_pcm);

#if 0
      static const unsigned table[12] = {0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14};

      midi_note_ -= 1;

      unsigned octave = midi_note_ / 12;
      unsigned note   = table[midi_note_ % 12];

      ym2151.setCh<YM2151::KC>(index_, (octave << 4) | note);

      ym2151.noteOn(index_);
#endif
   }

   void voiceOff(unsigned index_, uint8_t velocity_) override
   {
      // ym2151.noteOff(index_);
   }

   void voicePressure(unsigned index_, uint8_t level_) override
   {
   }

   void voicePitchBend(unsigned index_, int16_t value_) override
   {
   }

   void voiceControl(unsigned index_, uint8_t control_, uint8_t value_) override
   {
      switch(control_)
      {
      case 7:
         if (index_ == 0)
            audio.volume = value_;
         break;

      case 8:
         if (index_ == 0)
            audio.balance = value_;
         break;
      }
   }

   static const unsigned YM2151_CLOCK_HZ = 4000000; //!< 4 MHz

   SynthIO& io;

#if defined(HW_NATIVE)
   YM2151::Emulator ym2151{};
#else
   YM2151::Hardware<MTL::Pio0,
                    /* CTRL4    */ MTL::PIN_4,
                    /* CLK_M    */ MTL::PIN_9,
                    /* DATA8    */ MTL::PIN_14,
                    /* REV_DATA */ true> ym2151{};
#endif

   Decoder decoder{};
};

} // namespace VGM
