#pragma once

#include <cstdint>

#include <userver/utils/expected.hpp>

#include <modbus/parse_error.hpp>

namespace modbus {

enum class Coil : std::uint16_t {
    kOff = 0x00'00,
    kOn = 0xFF'00,
};

constexpr userver::utils::expected<Coil, ParseError> CoilFromRaw(std::uint16_t raw) noexcept {
    if (raw == static_cast<std::uint16_t>(Coil::kOn)) {
        return Coil::kOn;
    }

    if (raw == static_cast<std::uint16_t>(Coil::kOff)) {
        return Coil::kOff;
    }

    return userver::utils::unexpected{ParseError::kInvalidValue};
}

[[nodiscard]] constexpr std::uint16_t CoilToRaw(Coil coil) noexcept { return static_cast<std::uint16_t>(coil); }

}  // namespace modbus
