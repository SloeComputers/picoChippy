//-------------------------------------------------------------------------------
// Copyright (c) 2025 John D. Haughton
// SPDX-License-Identifier: MIT
//-------------------------------------------------------------------------------

// \brief Interface for OKI M6295

#pragma once

#include <cstdint>

#include "Chip.h"

#undef  DBG
#define DBG if (0) printf

namespace OKIM6295 {

class Interface : public Chip
{
public:
   Interface()
      : Chip("OKIM6295", NUM_CHANNELS, /* clock_ticks_per_sample */ 128)
   {
   }

   void writeReg(uint8_t data_)
   {
      DBG("WR %02X\n", data_);

      writeBus(data_);
   }

   //! Initialize samples
   void reset() override
   {
      for(unsigned id = 1; id <= MAX_SAMPLES; ++id)
      {
         sample_table[id - 1].clear();
      }
   }

   //! Add a section of ROM
   unsigned addSample(uint32_t hw_addr_, const uint8_t* ptr_, unsigned size_) override
   {
      if (hw_addr_ == 0)
      {
         for(unsigned id = 1; id < 127; ++id)
         {
            const uint8_t* entry = ptr_ + id * 8;

            uint32_t start = (entry[0] << 16) | (entry[1] << 8) | (entry[2]);
            uint32_t end   = (entry[3] << 16) | (entry[4] << 8) | (entry[5]);

            if (start >= end) break;

            sample_table[id - 1].declare(start, end);
         }
      }

      for(unsigned id = 1; id < 127; ++id)
      {
         sample_table[id - 1].link(hw_addr_, hw_addr_ + size_, ptr_);
      }

      return 0;
   }

   void write(uint16_t addr_, uint8_t data_) override
   {
      writeBus(data_);
   }

protected:
   //!
   const uint8_t* getStartPtr(unsigned id_) const
   {
      return sample_table[id_ - 1].getStartPtr();
   }

   //!
   const uint8_t* getEndPtr(unsigned id_) const
   {
      return sample_table[id_ - 1].getEndPtr();
   }

   //! Write a byte to the OKIM6295 bus
   virtual void writeBus(uint8_t value_) = 0;

   static const unsigned NUM_CHANNELS = 4;

private:
   class SampleData
   {
   public:
      SampleData() = default;

      //! Return a pointer that can be used with an offset of addr_ to access the data
      const uint8_t* getStartPtr() const { return ptr; }

      //! Return a pointer that can be used with an offset of addr_ to access the data
      const uint8_t* getEndPtr() const { return ptr + size; }

      //! Declare a sample
      void declare(uint32_t start_, uint32_t end_)
      {
         start = start_;
         size  = end_ - start_;
      }

      //! Link a sample to the data location
      void link(uint32_t base_, uint32_t end_, const uint8_t* sim_ptr_)
      {
         if ((start >= base_) && (start <= end_))
         {
            ptr = sim_ptr_ + (start - base_);
         }
      }

      //! Remove a sample
      void clear()
      {
         start = 0;
         size  = 0;
         ptr   = nullptr;
      }

   private:
      uint32_t       start{}; //!< Start address of sample data
      uint32_t       size{};  //!< Size (bytes)
      const uint8_t* ptr{};   //!< Pointer to actual data minus start
   };

   static const unsigned MAX_SAMPLES = 127;

   SampleData sample_table[MAX_SAMPLES];
};

} // namespace OKIM6295
