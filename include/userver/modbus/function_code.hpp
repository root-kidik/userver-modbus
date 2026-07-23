#pragma once

#include <cstdint>

namespace modbus {

enum class FunctionCode : std::uint8_t {
  kReadCoils = 0x01,
  kReadDiscreteInputs = 0x02,
  kReadHoldingRegisters = 0x03,
  kReadInputRegisters = 0x04,
  kWriteSingleCoil = 0x05,
  kWriteSingleHoldingRegister = 0x06,
  kWriteMultipleCoils = 0x0F,
  kWriteMultipleHoldingRegisters = 0x10,
};

} // namespace modbus
