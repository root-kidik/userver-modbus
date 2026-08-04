#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

#include <userver/utils/expected.hpp>

#include <modbus/coil.hpp>
#include <modbus/function_code.hpp>
#include <modbus/parse_error.hpp>
#include <modbus/utils.hpp>

namespace modbus::request::impl {

template <typename T, FunctionCode Code, std::uint16_t MaxQty, std::uint16_t MinQty = 1>
class WriteMultiple {
public:
    static constexpr std::uint16_t kMinQuantity{MinQty};
    static constexpr std::uint16_t kMaxQuantity{MaxQty};

    static constexpr FunctionCode kFunctionCode{Code};

    static userver::utils::expected<WriteMultiple, ParseError> Create(std::uint16_t address, std::span<const T> values)
        noexcept {
        if (values.size() < kMinQuantity || values.size() > kMaxQuantity) {
            return userver::utils::unexpected{ParseError::kInvalidQuantity};
        }

        const auto quantity = static_cast<std::uint16_t>(values.size());

        if (WouldAddressOverflow(address, quantity)) {
            return userver::utils::unexpected{ParseError::kAddressOverflow};
        }

        if constexpr (std::is_same_v<T, Coil>) {
            for (const auto val : values) {
                if (val != Coil::kOn && val != Coil::kOff) {
                    return userver::utils::unexpected{ParseError::kInvalidValue};
                }
            }
        }

        return WriteMultiple{address, values};
    }

    static userver::utils::expected<WriteMultiple, ParseError> Deserialize(std::span<const std::byte>& buffer
    ) noexcept {
        const auto function_code = ReadBe<std::uint8_t>(buffer);
        if (!function_code) {
            return userver::utils::unexpected{function_code.error()};
        }

        if (static_cast<FunctionCode>(*function_code) != kFunctionCode) {
            return userver::utils::unexpected{ParseError::kInvalidFunctionCode};
        }

        const auto address = ReadBe<std::uint16_t>(buffer);
        if (!address) {
            return userver::utils::unexpected{address.error()};
        }

        const auto quantity = ReadBe<std::uint16_t>(buffer);
        if (!quantity) {
            return userver::utils::unexpected{quantity.error()};
        }

        if (*quantity < kMinQuantity || *quantity > kMaxQuantity) {
            return userver::utils::unexpected{ParseError::kInvalidQuantity};
        }

        if (WouldAddressOverflow(*address, *quantity)) {
            return userver::utils::unexpected{ParseError::kAddressOverflow};
        }

        const auto byte_count = ReadBe<std::uint8_t>(buffer);
        if (!byte_count) {
            return userver::utils::unexpected{byte_count.error()};
        }

        const auto expected_byte_count = CalcByteCount(*quantity);
        if (*byte_count != expected_byte_count) {
            return userver::utils::unexpected{ParseError::kInvalidQuantity};
        }

        std::array<T, MaxQty> parsed_values;

        if constexpr (std::is_same_v<T, Coil>) {
            std::uint8_t current_byte = 0;
            for (std::size_t i = 0; i < *quantity; ++i) {
                const auto bit_idx = i % 8;
                if (bit_idx == 0) {
                    const auto byte_val = ReadBe<std::uint8_t>(buffer);
                    if (!byte_val) {
                        return userver::utils::unexpected{byte_val.error()};
                    }
                    current_byte = *byte_val;
                }

                const auto is_on = ((current_byte >> bit_idx) & 0x01) != 0;
                parsed_values[i] = is_on ? Coil::kOn : Coil::kOff;
            }
        } else {
            for (std::size_t i = 0; i < *quantity; ++i) {
                const auto val = ReadBe<T>(buffer);
                if (!val) {
                    return userver::utils::unexpected{val.error()};
                }
                parsed_values[i] = *val;
            }
        }

        return Create(*address, std::span<const T>{parsed_values.data(), *quantity});
    }

    userver::utils::expected<std::span<std::byte>, ParseError> Serialize(std::span<std::byte> out) const noexcept {
        const std::size_t required_size = GetEncodedSize();
        if (out.size() < required_size) {
            return userver::utils::unexpected{ParseError::kBufferTooShort};
        }

        std::ignore = WriteBe(out, static_cast<std::uint8_t>(kFunctionCode));
        std::ignore = WriteBe(out, address_);
        std::ignore = WriteBe(out, GetQuantity());
        std::ignore = WriteBe(out, GetByteCount());

        if constexpr (std::is_same_v<T, Coil>) {
            const auto byte_count = GetByteCount();
            for (std::size_t byte_idx = 0; byte_idx < byte_count; ++byte_idx) {
                std::uint8_t byte_val = 0;
                for (std::size_t bit_idx = 0; bit_idx < 8; ++bit_idx) {
                    const auto coil_idx = byte_idx * 8 + bit_idx;
                    if (coil_idx < quantity_ && values_[coil_idx] == Coil::kOn) {
                        byte_val |= static_cast<std::uint8_t>(1U << bit_idx);
                    }
                }
                std::ignore = WriteBe(out, byte_val);
            }
        } else {
            for (std::size_t i = 0; i < quantity_; ++i) {
                std::ignore = WriteBe(out, values_[i]);
            }
        }

        return out;
    }

    [[nodiscard]] std::uint16_t GetAddress() const noexcept { return address_; }

    [[nodiscard]] std::uint16_t GetQuantity() const noexcept { return quantity_; }

    [[nodiscard]] std::uint8_t GetByteCount() const noexcept { return CalcByteCount(quantity_); }

    [[nodiscard]] std::size_t GetEncodedSize() const noexcept { return 6 + GetByteCount(); }

    [[nodiscard]] std::span<const T> GetValues() const noexcept {
        return std::span<const T>{values_.data(), quantity_};
    }

private:
    WriteMultiple(std::uint16_t address, std::span<const T> values) noexcept
        : address_{address}, quantity_{static_cast<std::uint16_t>(values.size())} {
        std::copy(values.begin(), values.end(), values_.begin());
    }

    static std::uint8_t CalcByteCount(std::size_t quantity) noexcept {
        if constexpr (std::is_same_v<T, Coil>) {
            return static_cast<std::uint8_t>((quantity + 7) / 8);
        } else {
            return static_cast<std::uint8_t>(quantity * sizeof(T));
        }
    }

    std::uint16_t address_;
    std::uint16_t quantity_;

    std::array<T, MaxQty> values_;
};

}  // namespace modbus::request::impl
