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

// \brief Hardware Interface for SN76489

// XXX INCOMPLETE

#pragma once

#include "SN76489/Interface.h"

#include "MTL/Gpio.h"
#include "MTL/chip/PioClock.h"

namespace SN76489 {

template <typename PIO_SYNTH,
          unsigned PIN_CTRL3,       // First pin for _CE, _WE and READY
          unsigned PIN_DATA8,       // First pin for D0-D7
          bool     REV_DATA>        // true => rev data bits
class Hardware : public Interface
{
public:
   Hardware(unsigned pin_clk_)
      : pin_clk(pin_clk_)
   {
   }

   void setClock(unsigned clock_freq_) override
   {
      int sd = clock.download(pio, clock_freq_);
      pio.start(1 << sd);

      reset();
   }

   void reset() override
   {
      _ce = _wr = true;
      wait_ns(T_AH);

      Interface::reset();
   }

private:
   static uint8_t revBits(uint8_t value_)
   {
      value_ = ((value_ & 0xF0) >> 4) | ((value_ & 0x0F) << 4);
      value_ = ((value_ & 0xCC) >> 2) | ((value_ & 0x33) << 2);
      value_ = ((value_ & 0xAA) >> 1) | ((value_ & 0x55) << 1);

      return value_;
   }

   //! Wait for at least the given nano-seconds
   void wait_ns(unsigned nano_seconds_)
   {
      // TODO nano sleep for improved performance
      usleep((nano_seconds_ / 1000) + 1);
   }

   //! Write a byte to the SN76489 bus
   void writeBus(uint8_t value_) override
   {
   }

   void waitForReady() override
   {
   }

   static constexpr unsigned T_AS   = 10;    //!< Address setup (ns)
   static constexpr unsigned T_AH   = 10;    //!< Address hold (ns)
   static constexpr unsigned T_CW   = 100;   //!< Chip slecet wait (ns)
   static constexpr unsigned T_DS   = 50;    //!< Data write setup (ns)
   static constexpr unsigned T_DH   = 10;    //!< Data read/write hold (ns)
   static constexpr unsigned T_ACC  = 180;   //!< Read data access (ns)
   static constexpr unsigned T_INIT = 25000; //!< Chip initialisation (ns)

   unsigned pin_clk;

   //!< Bi-directional data bus
   MTL::Gpio::Out<8, PIN_DATA8> data8;

   //!< Control signals
   MTL::Gpio::Out<1, PIN_CTRL3+0> _ce;   //!< Chip select
   MTL::Gpio::Out<1, PIN_CTRL3+1> _wr;   //!< Write enable
   MTL::Gpio::In<1,  PIN_CTRL3+2> ready; //!< Ready

   MTL::PioClock clock{};  //! Clock out to SN76489
   PIO_SYNTH     pio{};    //! PIO instance
};

} // namespace SN76489
