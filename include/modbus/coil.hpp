#pragma once

#include <cstdint>

#include <userver/utils/expected.hpp>

#include <modbus/parse_error.hpp>

namespace modbus {

enum class Coil : std::uint8_t {
    kOff = 0,
    kOn = 1,
};

inline constexpr std::uint16_t kCoilOnRawValue{0xFF00};
inline constexpr std::uint16_t kCoilOffRawValue{0x0000};

constexpr userver::utils::expected<Coil, ParseError> CoilFromRaw(std::uint16_t raw) noexcept {
    if (raw == kCoilOnRawValue) {
        return Coil::kOn;
    }

    if (raw == kCoilOffRawValue) {
        return Coil::kOff;
    }

    return userver::utils::unexpected{ParseError::kInvalidValue};
}

[[nodiscard]] constexpr std::uint16_t CoilToRaw(Coil coil) noexcept {
    return (coil == Coil::kOn) ? kCoilOnRawValue : kCoilOffRawValue;
}

}  // namespace modbus
