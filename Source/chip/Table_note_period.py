#-------------------------------------------------------------------------------
# Copyright (c) 2023 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

""" Convert MIDI note (7 binary places) to period (32 binary places)"""

import math
import table

A4_FREQ = 440.0
A4_MIDI = 69

table.gen("note_period",
          func = lambda note7_7, x :
             int(0x100000000 * math.pow(2, (A4_MIDI - (note7_7 / 128)) / 12) / A4_FREQ),
          typename  = "int32_t",
          log2_size = 14,
          fmt       = '08x')
