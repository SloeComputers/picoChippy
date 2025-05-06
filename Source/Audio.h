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

#pragma once

#include <cstdint>

class Audio
{
public:
   Audio() = default;

   //! VGM player audio processing
   void process(int16_t& left, int16_t& right, int32_t pcm_l, int32_t pcm_r)
   {
      int32_t mix_l = left  + pcm_l;
      int32_t mix_r = right + pcm_r;

      int32_t bal_l = (mix_l * (128 - balance)) / 64;
      int32_t bal_r = (mix_r * balance) / 64;

      // TODO LOG volume
      left  = (bal_l * volume) / 256;
      right = (bal_r * volume) / 256;
   }

   volatile uint8_t balance{64};
   volatile uint8_t volume{127};
};
