//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

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
