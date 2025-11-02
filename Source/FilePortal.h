//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include "Hardware/FilePortal.h"

#include "VGM/Decoder.h"

class FilePortal : public hw::FilePortal
{
public:
   FilePortal(const char* label_, VGM::Decoder& decoder_)
      : hw::FilePortal(label_, "https://github.com/AnotherJohnH/picoChippy")
      , decoder(decoder_)
   {
      newFileBuffer(vgm_file, sizeof(vgm_file));
   }

private:
   //! Detector for VGM files, called with a pointer to the first 64 bytes in the file
   bool isNewFileInteresting(const uint8_t* buffer_) override
   {
      bool is_vgm = (buffer_[0] == 'V') &&
                    (buffer_[1] == 'g') &&
                    (buffer_[2] == 'm') &&
                    (buffer_[3] == ' ');

      // Pre-emptive stop to help avoid errors
      if (is_vgm)
         decoder.stop();

      return is_vgm;
   }

   //! Called when a VGM file download is complete
   void newFile(uint32_t size_) override
   {
      printf("New VGM file %u\n", size_);

      decoder.load(vgm_file);
   }

   uint8_t vgm_file[384 * 1024] = {};

   VGM::Decoder& decoder;
};
