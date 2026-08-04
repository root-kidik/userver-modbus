#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

#include <boost/endian/conversion.hpp>

#include <userver/utils/expected.hpp>

#include <modbus/parse_error.hpp>

namespace modbus {

template <typename T>
userver::utils::expected<void, ParseError> WriteBe(std::span<std::byte>& out, T value) noexcept {
    if (out.size() < sizeof(T)) {
        return userver::utils::unexpected{ParseError::kBufferTooShort};
    }

    const T value_be = boost::endian::native_to_big(value);
    std::memcpy(out.data(), &value_be, sizeof(T));

    out = out.subspan(sizeof(T));

    return {};
}

template <typename T>
userver::utils::expected<T, ParseError> ReadBe(std::span<const std::byte>& buffer) noexcept {
    if (buffer.size() < sizeof(T)) {
        return userver::utils::unexpected{ParseError::kBufferTooShort};
    }

    T value_be = 0;
    std::memcpy(&value_be, buffer.data(), sizeof(T));

    buffer = buffer.subspan(sizeof(T));

    return boost::endian::big_to_native(value_be);
}

inline constexpr std::uint32_t kMaxAddressSpace = 0x10000;

[[nodiscard]] constexpr bool WouldAddressOverflow(std::uint16_t address, std::uint16_t quantity) noexcept {
    return (static_cast<std::uint32_t>(address) + quantity) > kMaxAddressSpace;
}

[[nodiscard]] constexpr std::size_t BitsToBytes(std::size_t bits) noexcept { return (bits + 7) / 8; }

}  // namespace modbus
