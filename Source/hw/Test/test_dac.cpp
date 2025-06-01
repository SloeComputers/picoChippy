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

// \brief test DAC

#include "hw/hw.h"

#include "Table_sine.h"

static hw::Dac dac{};

static hw::Led led{};

int MTL_main()
{
   const unsigned SAMPLE_RATE = 48000;
   const unsigned FREQ        = 440;
   const uint32_t PHASE_INC   = ((FREQ << 22) / SAMPLE_RATE) << 10;
   const unsigned FRAC_BITS   = 32 - LOG2_TABLE_SINE_SIZE;

   printf("\nDAC Test\n");
   printf("Pulsed %u Hz sine wave, 0.5s on 0.5s off\n", FREQ);
   printf("Sample rate     = %u Hz\n", SAMPLE_RATE);
   printf("Phase inc       = %08x\n", PHASE_INC);
   printf("Phase frac bits = %u\n", FRAC_BITS);

   dac.start(SAMPLE_RATE);

   while(true)
   {
      uint32_t phase{0};

      led = true;

      for(unsigned i = 0; i < (SAMPLE_RATE / 2); ++i)
      {
         int16_t  sample      = table_sine[phase >> FRAC_BITS];
         uint32_t sample_pair = sample << 16 | (sample & 0xFFFF);

         phase += PHASE_INC;

         dac.push(sample_pair);
      }

      led = false;

      for(unsigned i = 0; i < (SAMPLE_RATE / 2); ++i)
      {
         dac.push(0);
      }
   }

   return 0;
}
