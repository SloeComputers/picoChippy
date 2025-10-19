//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

// \brief YM2151 VGM player for Raspberry Pi Pico

#include <cstdio>

#include "Hardware/picoX21H/Config.h"

#include "Synth.h"
#include "SynthIO.h"
#include "FilePortal.h"

#include "Table_vgm.h"

#include "Dac.h"
#include "SN76489/Emulator.h"
#include "SegaPCM/Emulator.h"
#include "OKIM6295/Emulator.h"
#include "YM2151/Hardware.h"
   
#include "VGM/Decoder.h"

// -----------------------------------------------------------------------------

static const bool MIDI_DEBUG = false;

static YM2151::Hardware   ym2151{};
static SegaPCM::Emulator  sega_pcm{};
static SN76489::Emulator  sn76489{};
static OKIM6295::Emulator oki_m6295{};
static VGM::Decoder       decoder{};

static FilePortal file_portal{"picoChippy", decoder};
static SynthIO    synth_io{};
static Synth      synth{synth_io};


// --- Audio out DAC -----------------------------------------------------------

static chip::DacImpl<hw::Dac> dac{62500};


// --- Physical MIDI -----------------------------------------------------------

static hw::PhysMidi phys_midi{};


// --- USB MIDI ----------------------------------------------------------------

#if defined(HW_USB_DEVICE)

static hw::UsbFileMidi usb{0x91C0, "picoChippy", file_portal};

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
   usb.attachInstrument(2, ym2151);
   usb.attachInstrument(3, sn76489);
   usb.attachInstrument(4, sega_pcm);
   usb.attachInstrument(5, oki_m6295);
#endif

   phys_midi.setDebug(MIDI_DEBUG);
   phys_midi.attachInstrument(1, synth);
   phys_midi.attachInstrument(2, ym2151);
   phys_midi.attachInstrument(3, sn76489);
   phys_midi.attachInstrument(4, sega_pcm);
   phys_midi.attachInstrument(5, oki_m6295);

   synth.mapAkaiMIDImix();

   decoder.plugDAC(&dac);
   decoder.plugYM2151(&ym2151);
   decoder.plugSN76489(&sn76489);
   decoder.plugSegaPCM(&sega_pcm);
   decoder.plugOKIM6295(&oki_m6295);
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
