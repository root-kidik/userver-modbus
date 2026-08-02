#pragma once

#include <cstdint>
#include <iterator>
#include <memory>
#include <numeric>

#include <boost/crc.hpp>

namespace modbus {

inline constexpr std::uint16_t kCrcBits{16};
inline constexpr std::uint16_t kCrcPolynomial{0x80'05};
inline constexpr std::uint16_t kCrcInitialValue{0xFF'FF};
inline constexpr std::uint16_t kCrcFinalXor{0};

using CrcType = boost::crc_optimal<kCrcBits, kCrcPolynomial, kCrcInitialValue, kCrcFinalXor, true, true>;

template <typename It>
[[nodiscard]] std::uint16_t CalcCrc(It begin, It end) noexcept {
    if (begin == end) {
        return kCrcInitialValue;
    }

    CrcType crc;
    crc.process_bytes(std::to_address(begin), static_cast<std::size_t>(std::distance(begin, end)));
    return crc.checksum();
}

template <typename It>
[[nodiscard]] std::uint8_t CalcLrc(It begin, It end) noexcept {
    const auto sum = std::accumulate(begin, end, std::uint8_t{0}, [](std::uint8_t acc, auto b) noexcept {
        return acc + static_cast<std::uint8_t>(b);
    });

    return static_cast<std::uint8_t>(~sum + 1);
}

}  // namespace modbus
