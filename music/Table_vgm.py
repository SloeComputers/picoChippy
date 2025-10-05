#!/usr/bin/env python3
#-------------------------------------------------------------------------------
# Copyright (c) 2025 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

import sys
import table

filename = sys.argv[1]

data = []

with open(filename, 'rb') as file:
   while True:
      byte = file.read(1)
      if byte == b'':
         break
      data.append(int.from_bytes(byte, byteorder='big', signed=False))

table.gen("vgm",
          func      = lambda i,x : data[i],
          typename  = "uint8_t",
          size      = len(data),
          fmt       = '02x')
