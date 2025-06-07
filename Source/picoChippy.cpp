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

#include "hw/hw.h"

#include "Synth.h"
#include "SynthIO.h"
#include "FilePortal.h"

#include "Table_vgm.h"

#include "YM2151/Emulator.h"
#include "SN76489/Emulator.h"
#include "SegaPCM/Emulator.h"
#include "OKIM6295/Emulator.h"
   
#include "VGM/Decoder.h"

// -----------------------------------------------------------------------------

static const bool MIDI_DEBUG = true;

#if not defined(HW_NATIVE)

#include "MTL/MTL.h"
#include "MTL/Pins.h"
#include "YM2151/Hardware.h"

static YM2151::Hardware<MTL::Pio0,
                        MTL::Pio1,
                        /* CTRL4    */ MTL::PIN_4,
                        /* DATA8    */ MTL::PIN_14,
                        /* REV_DATA */ true> ym2151{};
#else
static YM2151::Emulator ym2151{};
#endif

static hw::Dac dac{};

static SegaPCM::Emulator  sega_pcm{};
static SN76489::Emulator  sn76489{};
static OKIM6295::Emulator oki_m6295{};
static VGM::Decoder       decoder{};

static FilePortal    file_portal{"picoChippy", decoder};
static SynthIO       synth_io{};
static Synth         synth{synth_io};


// --- Physical MIDI -----------------------------------------------------------

static hw::PhysMidi phys_midi{};


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
   phys_midi.tick();

#if defined(HW_USB_DEVICE)
   usb.tick();
#endif
}


// -----------------------------------------------------------------------------

void startAudio()
{
   unsigned clock_hz = decoder.getClock();

#if not defined(HW_NATIVE)
   ym2151.init(clock_hz,
               /* CLK M       */ MTL::PIN_9,
               /* CLK SD SAM1 */ MTL::PIN_10);
#endif

   unsigned sample_rate_hz = clock_hz / 64;

   dac.start(sample_rate_hz);

#if not defined(HW_NATIVE)
   MTL_start_core(1, runDAC);
#endif
}

// --- Entry point -------------------------------------------------------------

int main()
{
   // Clear screen and cursor to home
   printf("\e[2J");
   printf("\e[1,1H");

   printf("\n");

   puts(file_portal.getReadme());

   synth_io.displayLCD(0, " Cambridge pico ");
   synth_io.displayLCD(1, " -*- Chippy -*- ");

   usleep(1000000);

   decoder.load(table_vgm);
   decoder.plugSN76489(&sn76489);
   decoder.plugYM2151(&ym2151);
   decoder.plugSegaPCM(&sega_pcm);
   decoder.plugOKIM6295(&oki_m6295);

   startAudio();

#if defined(HW_USB_DEVICE)
   usb.setDebug(MIDI_DEBUG);
   usb.attachInstrument(1, synth);
   usb.attachInstrument(2, sn76489);
   usb.attachInstrument(3, sega_pcm);
   usb.attachInstrument(4, ym2151);
#endif

   phys_midi.setDebug(MIDI_DEBUG);
   phys_midi.attachInstrument(1, synth);
   phys_midi.attachInstrument(2, sn76489);
   phys_midi.attachInstrument(3, sega_pcm);
   phys_midi.attachInstrument(4, ym2151);

   decoder.setTickFn(midiIn);

   while(true)
   {
      midiIn();

      led = synth.isAnyVoiceOn();
   }

   return 0;
}
