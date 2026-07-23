#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <userver/utils/expected.hpp>

#include <userver/modbus/parse_error.hpp>

namespace modbus {

namespace detail {

template <typename T>
constexpr std::uint8_t ToUint8(T val) noexcept {
    if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::byte>) {
        return std::to_integer<std::uint8_t>(val);
    } else {
        return static_cast<std::uint8_t>(val);
    }
}

template <typename OutputIt, typename T = std::uint8_t>
concept NothrowWritableIterator = requires(OutputIt it, T val) {
    requires noexcept(*it = val);
    requires noexcept(++it);
};

}  // namespace detail

template <typename OutputIt>
[[nodiscard]] OutputIt WriteBe(OutputIt out, std::uint8_t value) noexcept(detail::NothrowWritableIterator<OutputIt>) {
    *out = static_cast<std::uint8_t>(value);
    ++out;
    return out;
}

template <typename OutputIt>
[[nodiscard]] OutputIt WriteBe(OutputIt out, std::uint16_t value) noexcept(detail::NothrowWritableIterator<OutputIt>) {
    out = WriteBe(out, static_cast<std::uint8_t>(value >> 8));
    out = WriteBe(out, static_cast<std::uint8_t>(value & 0xFF));
    return out;
}

template <typename OutputIt>
[[nodiscard]] OutputIt WriteBe(OutputIt out, std::uint32_t value) noexcept(detail::NothrowWritableIterator<OutputIt>) {
    out = WriteBe(out, static_cast<std::uint8_t>((value >> 24) & 0xFF));
    out = WriteBe(out, static_cast<std::uint8_t>((value >> 16) & 0xFF));
    out = WriteBe(out, static_cast<std::uint8_t>((value >> 8) & 0xFF));
    out = WriteBe(out, static_cast<std::uint8_t>(value & 0xFF));
    return out;
}

template <typename T, typename InputIt>
requires(std::same_as<T, std::uint8_t> || std::same_as<T, std::uint16_t> || std::same_as<T, std::uint32_t>)
userver::utils::expected<T, ParseError> ReadBe(InputIt& first, InputIt last) noexcept {
    T result = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
        if (first == last) {
            return userver::utils::unexpected{ParseError::kBufferTooShort};
        }
        result = static_cast<T>((result << 8) | detail::ToUint8(*first));
        ++first;
    }
    return result;
}

inline constexpr std::uint32_t kMaxAddressSpace = 0x10000;

[[nodiscard]] inline bool WouldAddressOverflow(std::uint16_t address, std::uint16_t quantity) noexcept {
    return (static_cast<std::uint32_t>(address) + quantity) > kMaxAddressSpace;
}

[[nodiscard]] constexpr std::size_t BitsToBytes(std::size_t bits) noexcept { return (bits + 7) / 8; }

}  // namespace modbus
