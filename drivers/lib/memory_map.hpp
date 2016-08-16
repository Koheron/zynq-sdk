/// Memory map devices
///
/// (c) Koheron

#ifndef __DRIVERS_LIB_MEMORY_MAP_HPP__
#define __DRIVERS_LIB_MEMORY_MAP_HPP__

#include <cstdio>
#include <cstdint>
#include <tuple>
#include <memory>

extern "C" {
    #include <unistd.h>
    #include <sys/mman.h>
}

#include <drivers/addresses.hpp>

#define DEFAULT_MAP_SIZE 4096UL // = PAGE_SIZE
#define MAP_MASK(size) ((size) - 1)

// #define ASSERT_WRITABLE assert((protection & PROT_WRITE) == PROT_WRITE);
// #define ASSERT_READABLE assert((protection & PROT_READ) == PROT_READ);
#define ASSERT_WRITABLE
#define ASSERT_READABLE

typedef size_t MemMapID;

namespace addresses {

constexpr uint32_t count = address_array.size();

// Access elements in address_array

constexpr uintptr_t get_base_addr(MemMapID id) {
    return std::get<0>(address_array[id]);
}

// Makes sure it gets evaluated at compile time
static_assert(get_base_addr(CONFIG_ID) == std::get<0>(address_array[CONFIG_ID]), "get_base_address test failed");

constexpr uint32_t get_range(MemMapID id) {
    return std::get<1>(address_array[id]);
}

constexpr uint32_t get_protection(MemMapID id) {
    return std::get<2>(address_array[id]);
}

constexpr uint32_t get_n_blocks(MemMapID id) {
    return std::get<3>(address_array[id]);
}

constexpr uint32_t get_total_size(MemMapID id) {
    return get_range(id) * get_n_blocks(id);
}

} // namespace addresses

struct MemoryMapBase {
    MemoryMapBase(MemMapID id_)
    : id(id_) {}

    MemMapID id;
};

template<MemMapID id>
class MemoryMap : public MemoryMapBase
{
  public:
    MemoryMap(int *fd_);
    ~MemoryMap();

    int get_protection() const {return protection;}
    int get_status() const {return status;}
    uintptr_t get_base_addr() const {return base_address;}
    uint32_t mapped_size() const {return size;}
    uintptr_t get_phys_addr() const {return phys_addr;}

    std::tuple<uintptr_t, int, uintptr_t, uint32_t, int>
    get_params() {
        return std::make_tuple(
            base_address,
            status,
            phys_addr,
            size,
            protection
        );
    }

    ////////////////////////////////////////
    // Write functions
    ////////////////////////////////////////

    // Write a register (offset defined at compile-time)
    template<uint32_t offset, typename T = uint32_t>
    void write(T value) {
        ASSERT_WRITABLE
        *(volatile T *) (base_address + offset) = value;
    }

    // Write a register (offset defined at run-time)
    template<typename T = uint32_t>
    void write_offset(uint32_t offset, T value) {
        ASSERT_WRITABLE
        *(volatile T *) (base_address + offset) = value;
    }

    template<typename T = uint32_t, uint32_t offset = 0>
    void set_ptr(const T *data_ptr, uint32_t buff_size, uint32_t block_idx = 0) {
        ASSERT_WRITABLE
        uintptr_t addr = base_address + block_size * block_idx + offset;
        for (uint32_t i=0; i < buff_size; i++)
            *(volatile T *) (addr + sizeof(T) * i) = data_ptr[i];
    }

    template<typename T = uint32_t>
    void set_ptr_offset(uint32_t offset, const T *data_ptr, uint32_t buff_size) {
        ASSERT_WRITABLE
        uintptr_t addr = base_address + offset;
        for (uint32_t i=0; i < buff_size; i++)
            *(volatile T *) (addr + sizeof(T) * i) = data_ptr[i];
    }

    // Write a std::array (offset defined at compile-time)
    template<typename T, size_t N, uint32_t offset = 0>
    void write_array(const std::array<T, N> arr) {
        set_ptr<T, offset>(arr.data(), N);
    }

    // Write a std::array (offset defined at run-time)
    template<typename T, size_t N>
    void write_array_offset(uint32_t offset, const std::array<T, N> arr) {
        set_ptr_offset<T>(offset, arr.data(), N);
    }

    template<uint32_t offset, uint32_t mask, typename T = uint32_t>
    void write_mask(uint32_t value) {
        ASSERT_WRITABLE
        uintptr_t addr = base_address + offset;
        *(volatile uintptr_t *) addr = (*((volatile uintptr_t *) addr) & ~mask) | (value & mask);
    }

    void write_mask_offset(uint32_t offset, uint32_t mask, uint32_t value) {
        ASSERT_WRITABLE
        uintptr_t addr = base_address + offset;
        *(volatile uintptr_t *) addr = (*((volatile uintptr_t *) addr) & ~mask) | (value & mask);
    }

    ////////////////////////////////////////
    // Read functions
    ////////////////////////////////////////

    // Read a register (offset defined at compile-time)
    template<uint32_t offset, typename T = uint32_t>
    T read() {
        ASSERT_READABLE
        return *(volatile T *) (base_address + offset);
    }

    // Read a register (offset defined at run-time)
    template<typename T = uint32_t>
    T read_offset(uint32_t offset) {
        ASSERT_READABLE
        return *(volatile T *) (base_address + offset);
    }

    template<typename T = uint32_t, uint32_t offset = 0>
    T* get_ptr(uint32_t block_idx = 0) {
        ASSERT_READABLE
        return reinterpret_cast<T*>(base_address + block_size * block_idx + offset);
    }

    template<typename T = uint32_t>
    T* get_ptr_offset(uint32_t offset = 0) {
        ASSERT_READABLE
        return reinterpret_cast<T*>(base_address + offset);
    }

    // Read a std::array (offset defined at compile-time)
    template<typename T, size_t N, uint32_t offset = 0>
    std::array<T, N>& read_array(uint32_t block_idx = 0) {
        auto p = get_ptr<std::array<T, N>, offset>(block_idx);
        return *p;
    }

    // Read a std::array (offset defined at run-time)
    template<typename T, size_t N>
    std::array<T, N>& read_array_offset(uint32_t offset) {
        auto p = get_ptr_offset<std::array<T, N>>(offset);
        return *p;
    }

    ////////////////////////////////////////
    // Bit manipulation
    ////////////////////////////////////////

    // Set a bit (offset and index defined at compile-time)
    template<uint32_t offset, uint32_t index>
    void set_bit() {
        ASSERT_WRITABLE
        uintptr_t addr = base_address + offset;
        *(volatile uintptr_t *) addr = *((volatile uintptr_t *) addr) | (1 << index);
    }

    // Set a bit (offset and index defined at run-time)
    void set_bit_offset(uint32_t offset, uint32_t index) {
        ASSERT_WRITABLE
        uintptr_t addr = base_address + offset;
        *(volatile uintptr_t *) addr = *((volatile uintptr_t *) addr) | (1 << index);
    }

    // Clear a bit (offset and index defined at compile-time)
    template<uint32_t offset, uint32_t index>
    void clear_bit() {
        ASSERT_WRITABLE
        uintptr_t addr = base_address + offset;
        *(volatile uintptr_t *) addr = *((volatile uintptr_t *) addr) & ~(1 << index);
    }

    // Clear a bit (offset and index defined at run-time)
    void clear_bit_offset(uint32_t offset, uint32_t index) {
        ASSERT_WRITABLE
        uintptr_t addr = base_address + offset;
        *(volatile uintptr_t *) addr = *((volatile uintptr_t *) addr) & ~(1 << index);
    }

    // Toggle a bit (offset and index defined at compile-time)
    template<uint32_t offset, uint32_t index>
    void toggle_bit() {
        ASSERT_WRITABLE
        uintptr_t addr = base_address + offset;
        *(volatile uintptr_t *) addr = *((volatile uintptr_t *) addr) ^ (1 << index);
    }

    // Toggle a bit (offset and index defined at run-time)
    void toggle_bit_offset(uint32_t offset, uint32_t index) {
        ASSERT_WRITABLE
        uintptr_t addr = base_address + offset;
        *(volatile uintptr_t *) addr = *((volatile uintptr_t *) addr) ^ (1 << index);
    }

    // Read a bit (offset and index defined at compile-time)
    template<uint32_t offset, uint32_t index>
    bool read_bit() {
        ASSERT_READABLE
        return *((volatile uint32_t *) (base_address + offset)) & (1 << index);
    }

    // Read a bit (offset and index defined at run-time)
    bool read_bit_offset(uint32_t offset, uint32_t index) {
        ASSERT_READABLE
        return *((volatile uint32_t *) (base_address + offset)) & (1 << index);
    }

    enum Status {
        MEMMAP_CLOSED,       ///< Memory map closed
        MEMMAP_OPENED,       ///< Memory map opened
        MEMMAP_CANNOT_UMMAP, ///< Memory map cannot be unmapped
        MEMMAP_FAILURE       ///< Failure at memory mapping
    };

  private:
    int *fd;                 ///< /dev/mem file ID (Why is this a pointer ?)
    void *mapped_base;       ///< Map base address
    uintptr_t base_address;  ///< Virtual memory base address of the device
    int status;              ///< Status
    uintptr_t phys_addr = addresses::get_base_addr(id);
    uint32_t n_blocks = addresses::get_n_blocks(id);
    uint32_t block_size = addresses::get_range(id);
    uint32_t size = addresses::get_total_size(id);
    int protection = addresses::get_protection(id);
};

template<MemMapID id>
MemoryMap<id>*
cast_to_memory_map(const std::unique_ptr<MemoryMapBase>& memmap_base)
{
    return static_cast<MemoryMap<id>*>(memmap_base.get());
}

template<MemMapID id>
MemoryMap<id>::MemoryMap(int *fd_)
: MemoryMapBase(id)
, fd(fd_)
, mapped_base(nullptr)
, base_address(0)
, status(MEMMAP_CLOSED)
{
    mapped_base = mmap(0, size, protection, MAP_SHARED, *fd, phys_addr & ~MAP_MASK(size));

    if (mapped_base == (void *) -1) {
        fprintf(stderr, "Can't map the memory to user space.\n");
        status = MEMMAP_FAILURE;
        return;
    }

    status = MEMMAP_OPENED;
    base_address = (uintptr_t)mapped_base + (phys_addr & MAP_MASK(size));
}

template<MemMapID id>
MemoryMap<id>::~MemoryMap()
{
    if (status == MEMMAP_OPENED)
        munmap(mapped_base, size);
}

#endif // __DRIVERS_CORE_MEMORY_MAP_HPP__
