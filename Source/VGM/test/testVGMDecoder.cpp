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

#include "VGM/Decoder.h"

#include "STB/Test.h"

#include "SN76489/Emulator.h"
#include "YM2151/Emulator.h"
#include "SegaPCM/Emulator.h"

#include "Table_vgm.h"

TEST(VGMDecoder, basic)
{
   VGM::Decoder      decoder{};
   SN76489::Emulator sn76489{};
   YM2151::Emulator  ym2151{};
   SegaPCM::Emulator sega_pcm{};

   decoder.load(table_vgm);

   decoder.plugSN76489(&sn76489);
   decoder.plugYM2151(&ym2151);
   decoder.plugSegaPCM(&sega_pcm);

   decoder.play();
}
