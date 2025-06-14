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
#include "hw/YM2151.h"

#include "Synth.h"
#include "SynthIO.h"
#include "FilePortal.h"

#include "Table_vgm.h"

#include "Dac.h"
#include "SN76489/Emulator.h"
#include "SegaPCM/Emulator.h"
#include "OKIM6295/Emulator.h"
   
#include "VGM/Decoder.h"

// -----------------------------------------------------------------------------

static const bool MIDI_DEBUG = true;

static SegaPCM::Emulator  sega_pcm{};
static SN76489::Emulator  sn76489{};
static OKIM6295::Emulator oki_m6295{};
static VGM::Decoder       decoder{};

static FilePortal    file_portal{"picoChippy", decoder};
static SynthIO       synth_io{};
static Synth         synth{synth_io};


// --- FM Synth ----------------------------------------------------------------

static hw::YM2151 ym2151;


// --- Audio out DAC -----------------------------------------------------------

static chip::DacImpl<hw::Dac> dac{62500};


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

void SynthIO::playVGM(bool start_)
{
   if (start_)
      decoder.play();
   else
      decoder.stop();
}

// -----------------------------------------------------------------------------

#if not defined(HW_NATIVE)
static void runDAC()
{
   Sample mix_psg_pcm;
   Sample final_mix;

   // These synths are sampled at half the rate of the YM2151
   sn76489.setSampleMul(2);
   sega_pcm.setSampleMul(2);
   oki_m6295.setSampleMul(2);

   while(true)
   {
      decoder.sample();

      mix_psg_pcm = 0;
      sn76489.mixOut(mix_psg_pcm);
      sega_pcm.mixOut(mix_psg_pcm);
      oki_m6295.mixOut(mix_psg_pcm);

      final_mix = mix_psg_pcm;
      ym2151.mixOut(final_mix);
      dac.push(final_mix);

      final_mix = mix_psg_pcm;
      ym2151.mixOut(final_mix);
      dac.push(final_mix);
   }
}
#endif


// -----------------------------------------------------------------------------

void startAudio()
{
   unsigned clock_hz       = 4000000;
   unsigned sample_rate_hz = clock_hz / 64;

   ym2151.start(clock_hz);
   dac.setSampleRate(sample_rate_hz);

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

   puts(file_portal.genREADME());

   synth_io.displayLCD(0, " Cambridge pico ");
   synth_io.displayLCD(1, " -*- Chippy -*- ");

   usleep(1000000);

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

   decoder.plugDAC(&dac);
   decoder.plugSN76489(&sn76489);
   decoder.plugSegaPCM(&sega_pcm);
   decoder.plugOKIM6295(&oki_m6295);
   decoder.plugYM2151(&ym2151);
   decoder.load(table_vgm);

   while(true)
   {
      phys_midi.tick();

#if defined(HW_USB_DEVICE)
      usb.tick();
#endif

      decoder.tick();

      led = synth.isAnyVoiceOn();
   }

   return 0;
}
