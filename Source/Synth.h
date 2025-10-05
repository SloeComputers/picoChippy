//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdio>
#include <unistd.h>

#include "STB/MIDIInstrument.h"

#include "SynthIO.h"

class Synth : public MIDI::Instrument
{
public:
   Synth(SynthIO& synth_io_)
      : MIDI::Instrument(/* N */ 8)
      , io(synth_io_)
   {
   }

private:
   void voiceOn(unsigned index_, uint8_t midi_note_, uint8_t velocity_) override
   {
      if (midi_note_ == 60)
         io.playVGM(true);
      else if (midi_note_ == 61)
         io.playVGM(false);
   }

   SynthIO& io;
};
