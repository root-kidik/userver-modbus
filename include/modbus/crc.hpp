#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include <boost/crc.hpp>

namespace modbus {

inline constexpr std::uint16_t kCrcBits{16};
inline constexpr std::uint16_t kCrcPolynomial{0x80'05};
inline constexpr std::uint16_t kCrcInitialValue{0xFF'FF};
inline constexpr std::uint16_t kCrcFinalXor{0};

using CrcType = boost::crc_optimal<kCrcBits, kCrcPolynomial, kCrcInitialValue, kCrcFinalXor, true, true>;

[[nodiscard]] inline std::uint16_t CalcCrc(std::span<const std::byte> buffer) noexcept {
    if (buffer.empty()) {
        return kCrcInitialValue;
    }

    CrcType crc;
    crc.process_bytes(buffer.data(), buffer.size());
    return crc.checksum();
}

[[nodiscard]] inline std::uint8_t CalcLrc(std::span<const std::byte> buffer) noexcept {
    std::uint8_t sum{0};

    for (const auto b : buffer) {
        sum += static_cast<std::uint8_t>(b);
    }

    return static_cast<std::uint8_t>(~sum + 1);
}

}  // namespace modbus
