/// Init commands for all bitstreams
///
/// (c) Koheron

#ifndef __DRIVERS_INIT_HPP__
#define __DRIVERS_INIT_HPP__

#include <drivers/lib/memory_manager.hpp>
#include <drivers/memory.hpp>

class Init
{
  public:
    Init(DevMem& dvm_) {};

    void load_settings() {};
};

#endif // __DRIVERS_INIT_HPP__