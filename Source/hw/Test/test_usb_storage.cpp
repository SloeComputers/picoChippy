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
#include <cstring>

#include "STB/FAT/FAT16.h"

class FileSystem: public STB::FAT16<6>
{
public:
   FileSystem()
      : STB::FAT16<6>("HW_TEST")
   {
      static const char* readme_txt = "Hello, world!";

      addFile("README.txt", strlen(readme_txt), (uint8_t*)readme_txt);
   }
};

static FileSystem file_system;

#if defined(HW_USB_DEVICE)

static hw::USBStorageDevice usb{0x91C0, "HW-Test", file_system};

extern "C" void IRQ_USBCTRL() { usb.irq(); }

#endif

static hw::Led led{};

signed MTL_main()
{
   printf("\n\n-------------------------------\n");
   printf("TEST USB STORAGE\n\n");

   while(true)
   {
      usleep(50000);

      led = not led;
   }

   return 0;
}
