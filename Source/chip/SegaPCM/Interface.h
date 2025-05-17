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

// \brief Interface for SegaPCM 315-3218

#pragma once

#include <cstdint>

#include "Chip.h"

#undef  DBG
#define DBG if (0) printf

namespace SegaPCM {

class Interface : public Chip
{
public:
   Interface()
      : Chip("SegaPCM", NUM_CHANNELS)
   {
   }

   //! Initialize all channels and samples
   void reset()
   {
      for(unsigned channel = 0; channel < NUM_CHANNELS; ++channel)
      {
         setFreq(channel, 0);
         setVol(channel, 0, 0);
         noteOff(channel);
      }

      for(unsigned id = 1; id <= MAX_SAMPLES; ++id)
      {
         clrSample(id);
      }
   }

   bool isSampleIdValid(unsigned id_) const
   {
      if ((id_ == 0) || (id_ > MAX_SAMPLES))
         return false;

      return sample_list[id_ - 1].isValid();
   }

   //! Declare a sample and return an id
   unsigned addSample(uint32_t hw_addr_, const uint8_t* ptr_, unsigned size_)
   {
      for(unsigned i = 0; i < MAX_SAMPLES; ++i)
      {
         if (not sample_list[i].isValid())
         {
            sample_list[i].set(hw_addr_, ptr_, size_);
            return i + 1;
         }
      }

      return 0;
   }

   //! Remove a sample
   void clrSample(unsigned id_)
   {
      if (not isSampleIdValid(id_))
         return;

      sample_list[id_ - 1].clear();
   }

   //! Set channel frequency
   void setFreq(unsigned channel_, unsigned freq_)
   {
      uint16_t addr = (channel_ & CHANNEL_MASK) << 3;

      writeReg(addr + 0x07, freq_);
   }

   //! Set channel volume
   void setVol(unsigned channel_, unsigned lft7_, unsigned rgt7_)
   {
      uint16_t addr = (channel_ & CHANNEL_MASK) << 3;

      writeReg(addr + 0x02, lft7_);
      writeReg(addr + 0x03, rgt7_);
   }

   //! Set channel loop point (sample specific)
   void setLoopPoint(unsigned channel_, uint32_t hw_addr_)
   {
      uint16_t addr = (channel_ & CHANNEL_MASK) << 3;

      writeReg(addr + 0x04, hw_addr_);
      writeReg(addr + 0x05, hw_addr_ >> 8);
   }

   //! Play a sample
   void noteOn(unsigned channel_, unsigned id_, bool loop_ = false)
   {
      if (not isSampleIdValid(id_))
         return;

      const SampleData* sample = &sample_list[id_ - 1];
      uint16_t          addr   = (channel_ & CHANNEL_MASK) << 3;

      writeReg(addr + 0x84, sample->start);
      writeReg(addr + 0x85, sample->start >> 8);
      writeReg(addr + 0x06, sample->end >> 8);

      uint8_t mask = (sample->start & 0xF0000) >> 12;

      if (not loop_)
         mask |= 1<<1;

      writeReg(addr + 0x86, mask);
   }

   //! Stop a sample playing
   void noteOff(unsigned channel_)
   {
      uint16_t addr = (channel_ & CHANNEL_MASK) << 3;

      writeReg(addr + 0x86, 0b00000001);
   }

   void writeReg(uint16_t addr_, uint8_t data_)
   {
      DBG("WR %02X => %04X\n", data_, addr_);

      writeBus(addr_, data_);
   }

private:
   struct SampleData;

protected:
   //! Return a pointer that can be used to translate a sample address
   //! in the original hardware to a local pointer
   const SampleData* getSample(uint32_t hw_addr_) const
   {
      for(unsigned i = 0; i < MAX_SAMPLES; ++i)
      {
         const SampleData* sample = &sample_list[i];

         if (sample->contains(hw_addr_))
            return sample;
      }

      return nullptr;
   }

   //! Return a pointer that can be used to translate a sample address
   //! in the original hardware to a local pointer
   const uint8_t* getSamplePtr(uint32_t hw_addr_) const
   {
      const SampleData* sample = getSample(hw_addr_);
      if (sample != nullptr)
      {
         return sample->getPtr();
      }

      return nullptr;
   }

   //! Write a byte to the SegaPCM bus
   virtual void writeBus(uint16_t addr_, uint8_t value_) = 0;

   static const unsigned NUM_CHANNELS = 16;
   static const unsigned CHANNEL_MASK = NUM_CHANNELS - 1;

private:
   struct SampleData
   {
      SampleData() = default;

      //! Sample slot has been setup
      bool isValid() const { return end > start; }

      //! Hardware address is somwhere in this sample
      bool contains(uint32_t hw_addr_) const { return (hw_addr_ >= start) && (hw_addr_ < end); }

      //! Return a pointer that can be used with an offset of addr_ to access the data
      const uint8_t* getPtr() const { return ptr; }

      //! Setup a sample
      void set(uint32_t hw_addr_, const uint8_t* sim_ptr_, unsigned size_)
      {
         start = hw_addr_;
         end   = hw_addr_ + size_;
         ptr   = sim_ptr_ - hw_addr_;
      }

      //! Remove a sample
      void clear()
      {
         start = 0;
         end   = 0;
         ptr   = nullptr;
      }

      uint32_t       start{}; //!< Start address of sample data
      uint32_t       end{};   //!< End address of sample data
      const uint8_t* ptr{};   //!< Pointer to actual data minus start
   };

   static const unsigned MAX_SAMPLES = 64; //!< XXX how many is enough?

   SampleData sample_list[MAX_SAMPLES];
};

} // namespace SegaPCM
