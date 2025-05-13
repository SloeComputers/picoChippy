#!/usr/bin/env python3
#------------------------------------------------------------------------------
# Copyright (c) 2023 John D. Haughton
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#------------------------------------------------------------------------------

import math
import table
import sys

def midi_vol(midi_vol_7, x):
   """ Convert MIDI volume to a normalized gain """
   # Hard code zero => infinite attenuation 
   if midi_vol_7 == 0:
      return 0
   MAX_ATTEN_DB = 60
   atten_db = (127 - midi_vol_7) * MAX_ATTEN_DB / 126.0
   return int(math.pow(10, -atten_db / 20.0) * 0x7FFF)

def note_period(note7_7, x):
   """ Convert MIDI note (7 binary places) to period (32 binary places)"""
   A4_FREQ = 440.0
   A4_MIDI = 69
   return int(0x100000000 * math.pow(2, (A4_MIDI - (note7_7 / 128)) / 12) / A4_FREQ)

table.gen("midi_vol",
          func = midi_vol,
          typename = "int16_t",
          log2_size = 7,
          fmt = '04x')

table.gen("note_period",
          func = note_period,
          typename = "int32_t",
          log2_size = 14,
          fmt = '08x')
