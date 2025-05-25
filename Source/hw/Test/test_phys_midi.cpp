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

#include "hw/hw.h"

#include <cstdio>

static hw::PhysMidi midi{};

static hw::Led led{};

signed MTL_main()
{
   printf("\nMIDI Test\n");

   while(true)
   {
      uint8_t  byte = midi.rx();
      unsigned cmd  = byte >> 4;

      led = not led;

      if (cmd < 0x8)
      {
         // Ignore until sycnhronised
      }
      else if (cmd == 0xF)
      {
         // SYSTEM
      }
      else
      {
         unsigned channel = byte & 0xF;

         switch(cmd)
         {
         case 0x8: printf("CH%u NOTE OFF %3u %3u\n", channel, midi.rx(), midi.rx()); break;
         case 0x9: printf("CH%u NOTE ON  %3u %3u\n", channel, midi.rx(), midi.rx()); break;
         case 0xA: printf("CH%u NOTE PRE %3u %3u\n", channel, midi.rx(), midi.rx()); break;
         case 0xB: printf("CH%u CTRL     %3u %3u\n", channel, midi.rx(), midi.rx()); break;
         case 0xC: printf("CH%u PROG     %3u %3u\n", channel, midi.rx());            break;
         case 0xD: printf("CH%u PRES     %3u\n",     channel, midi.rx());            break;
         case 0xE: printf("CH%u PTCH     %d\n",      channel, midi.rx(), midi.rx()); break;

         default:
            break;
         }
      }
   }

   return 0;
}
