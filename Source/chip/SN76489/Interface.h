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

// \brief Interface for Texas Instruments SN76489

#pragma once

#include <cstdint>

#include "Chip.h"

#include "Table_note_period.h"

#undef  DBG
#define DBG if (0) printf

namespace SN76489 {

class Interface : public Chip
{
public:
   Interface()
      : Chip("SN76489", /* num_voices */ 3, /* ticks_per_sample */ 128)
   {
   }

   //! Configure shift register size and taps (for emulation)
   virtual void config(unsigned bits_, uint32_t taps_, uint8_t flags_) {}

   //! Initialize all voices
   virtual void reset()
   {
      for(unsigned voice = 0; voice < 3; ++voice)
      {
         setAtten(voice, ATTEN_INF);
         setPeriod( voice, 0x040);
      }

      setAtten(3, ATTEN_INF);
      selectNoise(NOISE_MID);
   }

   //! Set voice (0-2) square wave half period
   void setPeriod(unsigned voice_, unsigned period10_)
   {
      unsigned period_ls4 = period10_ & 0b1111;
      unsigned period_ms6 = (period10_ >> 4) & 0b111111;
      unsigned reg        = voice_ * 2;

      writeReg(0b10000000 | (reg << 4) | period_ls4);
      writeReg(period_ms6);
   }

   //! Set voice attenumation dB x 2
   void setAtten(unsigned voice_, uint8_t atten4_)
   {
      unsigned reg = 0b001 | (voice_ * 2);

      writeReg(0b10000000 | (reg << 4) | (atten4_ & 0b1111));
   }

   //! Set voice 3 to noise
   void selectNoise(unsigned rate2_)
   {
      writeReg(0b11100100 | (rate2_ & 0b11));
   }

   //! Set voice 3 to periodic
   void selectPeriodic(unsigned rate2_)
   {
      writeReg(0b11100000 | (rate2_ & 0b11));
   }

   //! Raw register interface
   void writeReg(uint8_t data_)
   {
      DBG("WR %02X\n", data_);

      writeBus(data_);
   }

//------------------------------------------------------------------------------
// Implement MIDI::Instrument

   //! Play a note
   void voiceOn(unsigned voice_, uint8_t note_, uint8_t velocity_) override
   {
      setAtten(voice_, (127 - velocity_) >> 3);
      midi_note[voice_] = note_;
      updatePitch(voice_);
   }

   //! Stop a note
   void voiceOff(unsigned voice_, uint8_t velocity_) override
   {
      setAtten(voice_, ATTEN_INF);
   }

   //! Set channel (0-3) attenuation
   void voicePressure(unsigned voice_, uint8_t pressure_) override
   {
      setAtten(voice_, (127 - pressure_) >> 3);
   }

   virtual void voicePitchBend(unsigned voice_, int16_t value_) override
   {
      midi_pitch_mod = value_;
      updatePitch(voice_);
   }

   static const uint8_t ATTEN_NONE = 0x0;
   static const uint8_t ATTEN_2DB  = 0x1;
   static const uint8_t ATTEN_INF  = 0xF;

   //! Rates for voice 3 (noise/periodic)
   static const uint8_t NOISE_LOW    = 0b00;
   static const uint8_t NOISE_MID    = 0b01;
   static const uint8_t NOISE_HI     = 0b10;
   static const uint8_t NOISE_FOLLOW = 0b11;

protected:
   //! Write a byte to the SN76489 bus
   virtual void writeBus(uint8_t value_) = 0;

private:
   void updatePitch(unsigned voice_)
   {
      uint32_t note_7            = (midi_note[voice_] << 7) + (midi_pitch_mod >> 6);
      uint32_t period_32         = table_note_period[note_7];
      uint32_t period_18         = (period_32 + 0x2000) >> 14;
      uint32_t clock_freq_div256 = clock_freq >> 8;
      unsigned period10          = (period_18 * clock_freq_div256 + 0x200) >> 16;

      if (period10 > 0x3FF)
         period10 = 0x3FF;

      setPeriod(voice_, period10);
   }

   unsigned clock_freq{4000000};
   int16_t  midi_pitch_mod{0};
   uint8_t  midi_note[3];
};

}
