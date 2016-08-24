/// Tests for the memory API
///
/// (c) Koheron

#ifndef __DRIVERS_TEST_MEMORY_HPP__
#define __DRIVERS_TEST_MEMORY_HPP__

#include <cmath>

#include <drivers/lib/memory_manager.hpp>
#include <drivers/memory.hpp>

#define ASSERT(...) \
    if (!(__VA_ARGS__)) return false;

class TestMemory
{
  public:
    TestMemory(MemoryManager& mm)
    : ram(mm.get<mem::rambuf>())
    {}

    // Write/read single registers

    bool write_read_u32() {
        ram.write<0>(42);
        ASSERT(ram.read<0>() == 42)

        ram.write<0>(4294967295);
        ASSERT(ram.read<0>() == 4294967295)

        return true;
    }

    bool write_read_reg_u32(uint32_t offset) {
        ram.write_reg(offset, 42);
        ASSERT(ram.read_reg(offset) == 42)

        ram.write_reg(offset, 4294967295);
        ASSERT(ram.read_reg(offset) == 4294967295)

        return true;
    }

    bool write_read_i16() {
        ram.write<0, int16_t>(-42);
        ASSERT(ram.read<0, int16_t>() == -42)
        ASSERT(ram.read<0, uint16_t>() != -42)

        ram.write<0, int16_t>(-32767);
        ASSERT(ram.read<0, int16_t>() == -32767)

        ram.write<0, int16_t>(32767);
        ASSERT(ram.read<0, int16_t>() == 32767)

        return true;
    }

    bool write_read_reg_i16(uint32_t offset) {
        ram.write_reg<int16_t>(offset, -42);
        ASSERT(ram.read_reg<int16_t>(offset) == -42)
        ASSERT(ram.read_reg<uint16_t>(offset) != -42)

        ram.write_reg<int16_t>(offset, -32767);
        ASSERT(ram.read_reg<int16_t>(offset) == -32767)

        ram.write_reg<int16_t>(offset, 32767);
        ASSERT(ram.read_reg<int16_t>(offset) == 32767)

        return true;
    }

    bool write_read_float() {
        ram.write<0, float>(3.1415926535897);
        ASSERT(fabs(ram.read<0, float>() - 3.1415926535897) < 1E-6)

        return true;
    }

    bool write_read_reg_float(uint32_t offset) {
        ram.write_reg<float>(offset, 3.1415926535897);
        ASSERT(fabs(ram.read_reg<float>(offset) - 3.1415926535897) < 1E-6)

        return true;
    }

    // Write/read consecutive registers

    bool write_read_u32_array() {
        std::array<uint32_t, 2048> arr;

        for (size_t i=0; i<arr.size(); i++)
            arr[i] = i * i;

        ram.write_array<uint32_t, 2048, 0>(arr);

        auto& arr_read = ram.read_array<uint32_t, 2048, 0>();

        for (size_t i=0; i<arr_read.size(); i++)
            ASSERT(arr_read[i] == i * i)

        return true;
    }

    bool write_read_reg_u32_array(uint32_t offset) {
        std::array<uint32_t, 2048> arr;

        for (size_t i=0; i<arr.size(); i++)
            arr[i] = i * i;

        ram.write_reg_array(offset, arr);

        auto& arr_read = ram.read_reg_array<uint32_t, 2048>(offset);

        for (size_t i=0; i<arr_read.size(); i++)
            ASSERT(arr_read[i] == i * i)

        return true;
    }

    bool write_read_float_array() {
        std::array<float, 2048> arr;

        for (size_t i=0; i<arr.size(); i++)
            arr[i] = log(static_cast<float>(i + 1));

        ram.write_array<float, 2048, 0>(arr);

        auto& arr_read = ram.read_array<float, 2048, 0>();

        for (size_t i=0; i<arr_read.size(); i++)
            ASSERT(fabs(arr_read[i] - log(static_cast<float>(i + 1))) < 1E-6)

        return true;
    }

    bool write_read_reg_float_array(uint32_t offset) {
        std::array<float, 2048> arr;

        for (size_t i=0; i<arr.size(); i++)
            arr[i] = log(static_cast<float>(i + 1));

        ram.write_reg_array(offset, arr);

        auto& arr_read = ram.read_reg_array<float, 2048>(offset);

        for (size_t i=0; i<arr_read.size(); i++)
            ASSERT(fabs(arr_read[i] - log(static_cast<float>(i + 1))) < 1E-6)

        return true;
    }

  private:
    MemoryMap<mem::rambuf>& ram;
};

#endif // __DRIVERS_TEST_MEMORY_HPP__