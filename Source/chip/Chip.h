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

// \brief Common interface for synth chips

#pragma once

#include <cstdint>

#include "STB/MIDIInstrument.h"

#include "Sample.h"

#undef  DBG
#define DBG if (0) printf

class Chip : public MIDI::Instrument
{
public:
   Chip(const char* name_,
        unsigned num_voices_,
        unsigned clock_ticks_per_sample_)
      : MIDI::Instrument(num_voices_, /* base_channel */ 0)
      , clock_ticks_per_sample(clock_ticks_per_sample_)
      , name(name_)
   {
      controlUpdate();
   }

   //! Set clock frequency (Hz)
   virtual bool setClock(unsigned clock_freq_hz_)
   {
      clock_freq_hz = clock_freq_hz_;
      mute          = clock_freq_hz_ == 0;

      return mute;
   }

   const char* getName() const { return name; }

   unsigned getSampleFreq() const { return clock_freq_hz / clock_ticks_per_sample; }

   bool isMute() const { return mute; }

protected:
   //! Common audio processing for mono synth chips
   void mixer(int16_t mono_, Sample& mix_)
   {
      mix_ += level * mono_;
   }

   //! Common audio processing for stero synth chips
   void mixer(const Sample& stereo_, Sample& mix_)
   {
      mix_ += level * stereo_;
   }

   //! Control a voice
   void voiceControl(unsigned voice_, uint8_t control_, uint8_t value_) override
   {
      if (voice_ != 0)
         return;

      switch(control_)
      {
      case 7:
         volume = value_;
         controlUpdate();
         break;

      case 8:
         balance = value_;
         controlUpdate();
         break;

      default:
         break;
      }
   }

   volatile bool mute{true};
   unsigned      clock_freq_hz{};
   unsigned      clock_ticks_per_sample{1};

private:
   void controlUpdate()
   {
      // TODO convert to a dB scale
      level.left  = volume << 8;
      level.right = volume << 8;
   }

   const char*     name{};
   uint8_t         volume{127};
   uint8_t         balance{64};
   volatile Sample level{};
};
