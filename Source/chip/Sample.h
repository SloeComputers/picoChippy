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

// \brief Stereo sample pair

#pragma once

#include <cstdint>

struct Sample
{
   Sample() = default;

   Sample(int16_t left_, int16_t right_)
      : left(left_)
      , right(right_)
   {
   }

   uint32_t pack() const
   {
      return (left << 16) | (right & 0xFFFF);
   }

   int16_t operator=(int32_t value_)
   {
      return left = right = value_;
   }

   void operator=(const Sample& that)
   {
      left  = that.left;
      right = that.right;
   }

   void operator+=(const Sample& that)
   {
      left  += that.left;
      right += that.right;
   }

   Sample operator*(const int32_t scalar_) volatile
   {
      return Sample((left * scalar_) >> 15, (right * scalar_) >> 15);
   }

   Sample operator*(const Sample& that) volatile
   {
      return Sample((left * that.left) >> 15, (right * that.right) >> 15);
   }

   int32_t left{};
   int32_t right{};
};
