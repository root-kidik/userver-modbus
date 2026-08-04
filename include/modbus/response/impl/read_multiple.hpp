#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

#include <userver/utils/expected.hpp>

#include <modbus/coil.hpp>
#include <modbus/discrete_input.hpp>
#include <modbus/function_code.hpp>
#include <modbus/parse_error.hpp>
#include <modbus/utils.hpp>

namespace modbus::response::impl {

template <typename T, FunctionCode Code, std::uint16_t MaxQty, std::uint16_t MinQty = 1>
class ReadMultiple {
public:
    static constexpr std::uint16_t kMinQuantity{MinQty};
    static constexpr std::uint16_t kMaxQuantity{MaxQty};

    static constexpr FunctionCode kFunctionCode{Code};

    static userver::utils::expected<ReadMultiple, ParseError> Create(std::span<const T> values) noexcept {
        const auto quantity = values.size();

        if (quantity < kMinQuantity || quantity > kMaxQuantity) {
            return userver::utils::unexpected{ParseError::kInvalidQuantity};
        }

        return ReadMultiple{values};
    }

    static userver::utils::expected<ReadMultiple, ParseError> Deserialize(
        std::span<const std::byte>& buffer,
        std::uint16_t expected_quantity
    ) noexcept {
        const auto function_code = ReadBe<std::uint8_t>(buffer);
        if (!function_code) {
            return userver::utils::unexpected{function_code.error()};
        }

        if (static_cast<FunctionCode>(*function_code) != kFunctionCode) {
            return userver::utils::unexpected{ParseError::kInvalidFunctionCode};
        }

        const auto byte_count = ReadBe<std::uint8_t>(buffer);
        if (!byte_count) {
            return userver::utils::unexpected{byte_count.error()};
        }

        std::array<T, MaxQty> parsed_values;
        std::uint16_t actual_quantity = 0;

        if constexpr (std::is_same_v<T, Coil> || std::is_same_v<T, DiscreteInput>) {
            if (expected_quantity < kMinQuantity || expected_quantity > kMaxQuantity) {
                return userver::utils::unexpected{ParseError::kInvalidQuantity};
            }

            const auto expected_bytes = BitsToBytes(expected_quantity);
            if (*byte_count != expected_bytes) {
                return userver::utils::unexpected{ParseError::kInvalidValue};
            }

            std::uint16_t bits_read = 0;
            for (std::size_t i = 0; i < *byte_count; ++i) {
                const auto current_byte = ReadBe<std::uint8_t>(buffer);
                if (!current_byte) {
                    return userver::utils::unexpected{current_byte.error()};
                }

                for (std::uint8_t bit = 0; bit < 8 && bits_read < expected_quantity; ++bit, ++bits_read) {
                    const bool is_set = (*current_byte & (1U << bit)) != 0;
                    if constexpr (std::is_same_v<T, Coil>) {
                        parsed_values[bits_read] = is_set ? Coil::kOn : Coil::kOff;
                    } else {
                        parsed_values[bits_read] = is_set ? DiscreteInput::kOn : DiscreteInput::kOff;
                    }
                }
            }
            actual_quantity = expected_quantity;
        } else {
            if (*byte_count % sizeof(std::uint16_t) != 0) {
                return userver::utils::unexpected{ParseError::kInvalidValue};
            }

            const auto register_count = static_cast<std::uint16_t>(*byte_count / sizeof(std::uint16_t));
            if (register_count < kMinQuantity || register_count > kMaxQuantity) {
                return userver::utils::unexpected{ParseError::kInvalidQuantity};
            }

            for (std::size_t i = 0; i < register_count; ++i) {
                const auto reg = ReadBe<std::uint16_t>(buffer);
                if (!reg) {
                    return userver::utils::unexpected{reg.error()};
                }
                parsed_values[i] = *reg;
            }
            actual_quantity = register_count;
        }

        return Create(std::span<const T>{parsed_values.data(), actual_quantity});
    }

    userver::utils::expected<std::span<std::byte>, ParseError> Serialize(std::span<std::byte> out) const noexcept {
        const std::size_t required_size = GetEncodedSize();
        if (out.size() < required_size) {
            return userver::utils::unexpected{ParseError::kBufferTooShort};
        }

        std::ignore = WriteBe(out, static_cast<std::uint8_t>(kFunctionCode));
        std::ignore = WriteBe(out, GetByteCount());

        if constexpr (std::is_same_v<T, Coil> || std::is_same_v<T, DiscreteInput>) {
            std::uint8_t current_byte = 0;
            std::uint8_t bit_index = 0;

            for (std::size_t i = 0; i < quantity_; ++i) {
                const auto& val = values_[i];
                bool is_set = false;
                if constexpr (std::is_same_v<T, Coil>) {
                    is_set = (val == Coil::kOn);
                } else {
                    is_set = (val == DiscreteInput::kOn);
                }

                if (is_set) {
                    current_byte |= static_cast<std::uint8_t>(1U << bit_index);
                }

                ++bit_index;
                if (bit_index == 8) {
                    std::ignore = WriteBe(out, current_byte);
                    current_byte = 0;
                    bit_index = 0;
                }
            }

            if (bit_index > 0) {
                std::ignore = WriteBe(out, current_byte);
            }
        } else {
            for (std::size_t i = 0; i < quantity_; ++i) {
                std::ignore = WriteBe(out, values_[i]);
            }
        }

        return out;
    }

    [[nodiscard]] constexpr std::uint8_t GetByteCount() const noexcept {
        if constexpr (std::is_same_v<T, Coil> || std::is_same_v<T, DiscreteInput>) {
            return static_cast<std::uint8_t>(BitsToBytes(quantity_));
        } else {
            return static_cast<std::uint8_t>(quantity_ * sizeof(T));
        }
    }

    [[nodiscard]] constexpr std::size_t GetEncodedSize() const noexcept { return 2 + GetByteCount(); }

    [[nodiscard]] constexpr std::uint16_t GetQuantity() const noexcept { return quantity_; }

    [[nodiscard]] constexpr std::span<const T> GetValues() const noexcept {
        return std::span<const T>{values_.data(), quantity_};
    }

private:
    ReadMultiple(std::span<const T> values) noexcept : quantity_{static_cast<std::uint16_t>(values.size())} {
        std::copy(values.begin(), values.end(), values_.begin());
    }

    std::uint16_t quantity_;

    std::array<T, MaxQty> values_;
};

}  // namespace modbus::response::impl
