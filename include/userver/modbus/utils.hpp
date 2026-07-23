#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>

#include <userver/utils/expected.hpp>

namespace modbus {

enum class Error {
  kBufferTooShort,
  kExtraDataAtEnd,
  kInvalidFunctionCode,
  kInvalidQuantity,
  kAddressOverflow,
};

template <typename OutputIt>
OutputIt WriteBe(OutputIt out, std::uint8_t value) {
  *out = static_cast<std::byte>(value);
  out++;
  return out;
}

template <typename OutputIt>
OutputIt WriteBe(OutputIt out, std::uint16_t value) {
  *out = static_cast<std::byte>(value >> 8);
  out++;
  *out = static_cast<std::byte>(value & 0xFF);
  out++;
  return out;
}

template <typename OutputIt>
OutputIt WriteBe(OutputIt out, std::uint32_t value) {
  *out = static_cast<std::byte>((value >> 24) & 0xFF);
  out++;
  *out = static_cast<std::byte>((value >> 16) & 0xFF);
  out++;
  *out = static_cast<std::byte>((value >> 8) & 0xFF);
  out++;
  *out = static_cast<std::byte>(value & 0xFF);
  out++;
  return out;
}

namespace detail {

template <typename T, typename InputIt>
userver::utils::expected<T, Error> ReadBeImpl(InputIt &first,
                                              InputIt last) noexcept {
  T result = 0;
  for (std::size_t i = 0; i < sizeof(T); i++) {
    if (first == last) {
      return userver::utils::unexpected{Error::kBufferTooShort};
    }
    result = static_cast<T>((result << 8) | static_cast<std::uint8_t>(*first));
    first++;
  }
  return result;
}

} // namespace detail

template <typename T, typename InputIt>
  requires std::same_as<T, std::uint8_t>
userver::utils::expected<std::uint8_t, Error> ReadBe(InputIt &first,
                                                     InputIt last) noexcept {
  return detail::ReadBeImpl<std::uint8_t>(first, last);
}

template <typename T, typename InputIt>
  requires std::same_as<T, std::uint16_t>
userver::utils::expected<std::uint16_t, Error> ReadBe(InputIt &first,
                                                      InputIt last) noexcept {
  return detail::ReadBeImpl<std::uint16_t>(first, last);
}

template <typename T, typename InputIt>
  requires std::same_as<T, std::uint32_t>
userver::utils::expected<std::uint32_t, Error> ReadBe(InputIt &first,
                                                      InputIt last) noexcept {
  return detail::ReadBeImpl<std::uint32_t>(first, last);
}

inline constexpr std::uint32_t kMaxAddressSpace = 0x10000;

inline bool WouldAddressOverflow(std::uint16_t address,
                                 std::uint16_t quantity) {
  return (static_cast<std::uint32_t>(address) + quantity) > kMaxAddressSpace;
}

} // namespace modbus
