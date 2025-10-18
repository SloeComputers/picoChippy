//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

#pragma once

#include <cstdio>
#include <cstring>

#include "STB/FAT/FAT16.h"

#include "hw/hw.h"

#include "VGM/Decoder.h"

class FilePortal : public STB::FAT16<6>
{
public:
   FilePortal(const char* label_, VGM::Decoder& decoder_)
      : STB::FAT16<6>(label_)
      , decoder(decoder_)
   {
      addFile("INDEX.htm", strlen(index_html), (uint8_t*)index_html);

      newFileBuffer(vgm_file, sizeof(vgm_file));
   }

   //! Auto-generate the project README
   const char* genREADME()
   {
      char* s     = readme;
      char* end_s = readme + sizeof(readme);

      s += snprintf(s, end_s - s, "Program  : Cambridge pico Chippy (%s)\n", HW_DESCR);
      s += snprintf(s, end_s - s, "Author   : Copyright (c) 2025 John D. Haughton\n");
      s += snprintf(s, end_s - s, "License  : MIT\n");
      s += snprintf(s, end_s - s, "Version  : %s\n", PLT_VERSION);
      s += snprintf(s, end_s - s, "Commit   : %s\n", PLT_COMMIT);
      s += snprintf(s, end_s - s, "Built    : %s %s\n", __TIME__, __DATE__);
#if defined(__clang__)
      s += snprintf(s, end_s - s, "Compiler : Clang %s\n", __VERSION__);
#elif defined(__GNUC__)
      s += snprintf(s, end_s - s, "Compiler : GCC %s\n", __VERSION__);
#else
      s += snprintf(s, end_s - s, "Compiler : %s\n", __VERSION__);
#endif

#if not defined(HW_NATIVE)
      s += MTL::config.format(s, end_s - s);
#endif

      addFile("README.txt", s - readme, (uint8_t*)readme);

      return readme;
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

   char readme[2048];

   const char* index_html =
      "<html>"
         "<head>"
            "<meta http-equiv=\"refresh\" content=\"0;URL='https://github.com/AnotherJohnH/picoChippy'\"/>"
         "</head>"
         "<body>"
            "Redirecting to <a href='https://github.com/AnotherJohnH/picoChippy'>github.com</a>"
         "</body>"
      "</html>";

   uint8_t vgm_file[384 * 1024] = {};

   VGM::Decoder& decoder;
};
