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

inline constexpr std::uint8_t kErrorFlag = 0x80;
inline constexpr std::uint8_t kFunctionCodeMask = 0x7F;

[[nodiscard]] constexpr std::uint8_t ToErrorFunctionCode(std::uint8_t function_code) noexcept {
    return function_code | kErrorFlag;
}

[[nodiscard]] constexpr bool IsErrorFunctionCode(std::uint8_t function_code) noexcept {
    return (function_code & kErrorFlag) != 0;
}

[[nodiscard]] constexpr std::uint8_t ToNormalFunctionCode(std::uint8_t error_function_code) noexcept {
    return error_function_code & kFunctionCodeMask;
}

}  // namespace modbus
