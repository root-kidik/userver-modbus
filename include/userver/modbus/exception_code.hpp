#pragma once

#include <cstdint>

namespace modbus {

enum class ExceptionCode : std::uint8_t {
    kIllegalFunction = 0x01,
    kIllegalDataAddress = 0x02,
    kIllegalDataValue = 0x03,
    kServerDeviceFailure = 0x04,
    kAcknowledge = 0x05,
    kServerDeviceBusy = 0x06,
    kMemoryParityError = 0x08,
    kGatewayPathUnavailable = 0x0A,
    kGatewayTargetFailed = 0x0B,
};

inline constexpr std::uint8_t kErrorFlag = 0x80;
inline constexpr std::uint8_t kFunctionCodeMask = 0x7F;

[[nodiscard]] constexpr bool IsValidExceptionCode(std::uint8_t raw_code) noexcept {
    switch (static_cast<ExceptionCode>(raw_code)) {
        case ExceptionCode::kIllegalFunction:
        case ExceptionCode::kIllegalDataAddress:
        case ExceptionCode::kIllegalDataValue:
        case ExceptionCode::kServerDeviceFailure:
        case ExceptionCode::kAcknowledge:
        case ExceptionCode::kServerDeviceBusy:
        case ExceptionCode::kMemoryParityError:
        case ExceptionCode::kGatewayPathUnavailable:
        case ExceptionCode::kGatewayTargetFailed:
            return true;
        default:
            return false;
    }
}

[[nodiscard]] constexpr bool IsValidExceptionCode(ExceptionCode code) noexcept {
    return IsValidExceptionCode(static_cast<std::uint8_t>(code));
}

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
