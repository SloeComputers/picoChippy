#-------------------------------------------------------------------------------
# Copyright (c) 2025 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

import math
import table

MAX_LEVEL = 0x7fff
STEP_DB   = 3

def atten4(atten, x):
   """ Convert 4-bit attenuation (3dB) steps to normalise gain """
   # Hard code zero => infinite attenuation 
   if atten == 15:
      return 0
   atten_db = atten * STEP_DB
   return int(math.pow(10, -atten_db / 20.0) * MAX_LEVEL)

table.gen("atten4_3db",
          func = atten4,
          typename = "int16_t",
          log2_size = 4,
          fmt = '04x')
