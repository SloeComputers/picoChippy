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

// \brief YM2151 VGM player for Raspberry Pi Pico

#include <cstdio>

#include "Synth.h"
#include "SynthIO.h"
#include "FilePortal.h"

#include "hw/hw.h"

#include "Table_vgm.h"

#include "YM2151/Emulator.h"
#include "SN76489/Emulator.h"
#include "SegaPCM/Emulator.h"
   
#include "VGM/Decoder.h"

#if not defined(HW_NATIVE)

#include "MTL/MTL.h"
#include "MTL/Pins.h"
   
#include "MTL/chip/PioI2S_S16.h"
   
#include "YM2151/Hardware.h"

#endif


// -----------------------------------------------------------------------------

static const bool MIDI_DEBUG = true;

#if not defined(HW_NATIVE)
static YM2151::Hardware<MTL::Pio0,
                        MTL::Pio1,
                        /* CTRL4    */ MTL::PIN_4,
                        /* DATA8    */ MTL::PIN_14,
                        /* REV_DATA */ true> ym2151{};

static MTL::PioI2S_S16<MTL::Pio0> dac{};
#else
static YM2151::Emulator ym2151{};
#endif

static SegaPCM::Emulator sega_pcm{};
static SN76489::Emulator sn76489{};
static VGM::Decoder      decoder{};

static FilePortal    file_portal{"picoChippy", decoder};
static SynthIO       synth_io{};
static Synth         synth{synth_io};


// --- Physical MIDI -----------------------------------------------------------

static hw::MidiIn midi_in{};


// --- USB MIDI ----------------------------------------------------------------

#if defined(HW_USB_DEVICE)

static hw::USBDevice usb{0x91C0, "picoChippy", file_portal};

extern "C" void IRQ_USBCTRL() { usb.irq(); }

#endif


// --- 16x2 LCD display --------------------------------------------------------

#if not defined(HW_LCD_NONE)

static hw::Lcd lcd{};

#endif

void SynthIO::displayLCD(unsigned row, const char* text)
{
#if not defined(HW_LCD_NONE)
   lcd.move(0, row);
   lcd.print(text);
#endif
}


// --- LED ---------------------------------------------------------------------

static hw::Led led{};


// -----------------------------------------------------------------------------

void SynthIO::triggerVGM()
{
   decoder.play();
}

// -----------------------------------------------------------------------------

#if not defined(HW_NATIVE)

static void runDAC()
{
   Sample mix_psg_pcm;
   Sample final_mix;

   while(true)
   {
      decoder.tick();

      mix_psg_pcm = 0;
      sn76489.mixOut(mix_psg_pcm);
      sega_pcm.mixOut(mix_psg_pcm);

      final_mix = mix_psg_pcm;
      ym2151.mixOut(final_mix);
      dac.push(final_mix.pack());

      final_mix = mix_psg_pcm;
      ym2151.mixOut(final_mix);
      dac.push(final_mix.pack());
   }
}

#endif


// -----------------------------------------------------------------------------

static void midiIn(void* ptr = nullptr)
{
   midi_in.tick();

#if defined(HW_MIDI_USB_DEVICE)
   midi_usb.tick();
#endif
}


// -----------------------------------------------------------------------------

void startAudio()
{
#if not defined(HW_NATIVE)
   unsigned clock_hz = decoder.getClock();

   ym2151.init(clock_hz,
               /* CLK M       */ MTL::PIN_9,
               /* CLK SD SAM1 */ MTL::PIN_10);

   dac.download(clock_hz, /* SD */ MTL::PIN_31, /* LRCLK SCLK */ MTL::PIN_32);
   dac.start();

   MTL_start_core(1, runDAC);
#endif
}

// -----------------------------------------------------------------------------

int main()
{
   // Clear screen and cursor to home
   printf("\e[2J");
   printf("\e[1,1H");

   printf("\n");
   printf("Program  : Cambridge pico Chippy (%s)\n", HW_DESCR);
   printf("Author   : Copyright (c) 2025 John D. Haughton\n");
   printf("License  : MIT\n");
   printf("Version  : %s\n", PLT_VERSION);
   printf("Commit   : %s\n", PLT_COMMIT);
   printf("Built    : %s %s\n", __TIME__, __DATE__);
   printf("Compiler : %s\n", __VERSION__);
   printf("\n");

   synth_io.displayLCD(0, " Cambridge pico ");
   synth_io.displayLCD(1, " -*- Chippy -*- ");

   usleep(1000000);

   decoder.load(table_vgm);
   decoder.plugSN76489(&sn76489);
   decoder.plugYM2151(&ym2151);
   decoder.plugSegaPCM(&sega_pcm);

   startAudio();

#if defined(HW_MIDI_USB_DEVICE)
   midi_usb.setDebug(MIDI_DEBUG);
   midi_usb.attachInstrument(1, synth);
   midi_usb.attachInstrument(2, sn76489);
   midi_usb.attachInstrument(3, sega_pcm);
   midi_usb.attachInstrument(4, ym2151);
#endif

   midi_in.setDebug(MIDI_DEBUG);
   midi_in.attachInstrument(1, synth);
   midi_in.attachInstrument(2, sn76489);
   midi_in.attachInstrument(3, sega_pcm);
   midi_in.attachInstrument(4, ym2151);

   decoder.setTickFn(midiIn);

   while(true)
   {
      midiIn();

      led = synth.isAnyVoiceOn();
   }

   return 0;
}

