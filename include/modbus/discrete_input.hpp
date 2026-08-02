#pragma once

#include <cstdint>

#include <modbus/parse_error.hpp>

namespace modbus {

enum class DiscreteInput : std::uint8_t {
    kOff = 0,
    kOn = 1,
};

}  // namespace modbus
