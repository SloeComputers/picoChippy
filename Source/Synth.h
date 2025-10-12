//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdio>
#include <unistd.h>

#include "STB/MIDIconst.h"
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

   //! Remap MIDI control to another channel and control
   void mapMIDICtrl(uint8_t ctrl_in_, uint8_t chan_out_, uint8_t ctrl_out_)
   {
      ctrl_map[ctrl_in_].chan = chan_out_;
      ctrl_map[ctrl_in_].ctrl = ctrl_out_;
   }

   //! Remap AKAI MIDImix controls to the volume and pan on the output channels
   void mapAkaiMIDImix()
   {
      // AKAI chan #1
      mapMIDICtrl(18, 2, MIDI::CTRL_PAN);
      mapMIDICtrl(19, 2, MIDI::CTRL_VOLUME);

      // AKAI chan #2
      mapMIDICtrl(22, 3, MIDI::CTRL_PAN);
      mapMIDICtrl(23, 3, MIDI::CTRL_VOLUME);

      // AKAI chan #3
      mapMIDICtrl(26, 4, MIDI::CTRL_PAN);
      mapMIDICtrl(27, 4, MIDI::CTRL_VOLUME);

      // AKAI chan #4
      mapMIDICtrl(30, 5, MIDI::CTRL_PAN);
      mapMIDICtrl(31, 5, MIDI::CTRL_VOLUME);

      // AKAI chan #5
      mapMIDICtrl(48, 6, MIDI::CTRL_PAN);
      mapMIDICtrl(49, 6, MIDI::CTRL_VOLUME);

      // AKAI chan #6
      mapMIDICtrl(52, 7, MIDI::CTRL_PAN);
      mapMIDICtrl(53, 7, MIDI::CTRL_VOLUME);

      // AKAI chan #7
      mapMIDICtrl(56, 8, MIDI::CTRL_PAN);
      mapMIDICtrl(57, 8, MIDI::CTRL_VOLUME);

      // AKAI chan #8
      mapMIDICtrl(60, 9, MIDI::CTRL_PAN);
      mapMIDICtrl(61, 9, MIDI::CTRL_VOLUME);
   }

private:
   void voiceOn(unsigned index_, uint8_t midi_note_, uint8_t velocity_) override
   {
      if (midi_note_ == 60)
         io.playVGM(true);
      else if (midi_note_ == 61)
         io.playVGM(false);
   }

   void voiceControl(unsigned voice_, uint8_t control_, uint8_t value_) override
   {
      const CtrlMap& mapping = ctrl_map[control_];

      if (mapping.chan != 0)
      {
         interface->controlChange(mapping.chan, mapping.ctrl, value_);
      }
   }

   SynthIO& io;

   struct CtrlMap
   {
      CtrlMap() = default;

      uint8_t chan{0};
      uint8_t ctrl{0};
   };

   CtrlMap ctrl_map[128];
};
