#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include <userver/modbus/coil.hpp>
#include <userver/modbus/function_code.hpp>
#include <userver/modbus/parse_error.hpp>
#include <userver/modbus/utils.hpp>

#include <userver/utils/expected.hpp>

namespace modbus::response::impl {

template <typename T, FunctionCode Code, std::uint16_t MaxQty, std::uint16_t MinQty = 1>
class ReadMultiple {
public:
    static constexpr std::uint16_t kMinQuantity{MinQty};
    static constexpr std::uint16_t kMaxQuantity{MaxQty};

    static constexpr FunctionCode kFunctionCode{Code};

    static userver::utils::expected<ReadMultiple, ParseError> Create(std::vector<T> values) {
        const auto quantity = values.size();
        if (quantity < kMinQuantity || quantity > kMaxQuantity) {
            return userver::utils::unexpected{ParseError::kInvalidQuantity};
        }

        if constexpr (std::is_same_v<T, Coil>) {
            for (const auto val : values) {
                if (val != Coil::kOn && val != Coil::kOff) {
                    return userver::utils::unexpected{ParseError::kInvalidValue};
                }
            }
        }

        return ReadMultiple{std::move(values)};
    }

    template <typename InputIt>
    static userver::utils::expected<ReadMultiple, ParseError> Deserialize(
        InputIt& first,
        InputIt last,
        std::uint16_t expected_quantity
    ) {
        const auto function_code = ReadBe<std::uint8_t>(first, last);
        if (!function_code) {
            return userver::utils::unexpected{function_code.error()};
        }

        if (static_cast<FunctionCode>(*function_code) != kFunctionCode) {
            return userver::utils::unexpected{ParseError::kInvalidFunctionCode};
        }

        const auto byte_count = ReadBe<std::uint8_t>(first, last);
        if (!byte_count) {
            return userver::utils::unexpected{byte_count.error()};
        }

        std::vector<T> values;

        if constexpr (std::is_same_v<T, Coil> || std::is_same_v<T, bool>) {
            if (expected_quantity < kMinQuantity || expected_quantity > kMaxQuantity) {
                return userver::utils::unexpected{ParseError::kInvalidQuantity};
            }

            const auto expected_bytes = BitsToBytes(expected_quantity);
            if (*byte_count != expected_bytes) {
                return userver::utils::unexpected{ParseError::kInvalidValue};
            }

            values.reserve(expected_quantity);

            std::uint16_t bits_read = 0;
            for (std::size_t i = 0; i < *byte_count; ++i) {
                const auto current_byte = ReadBe<std::uint8_t>(first, last);
                if (!current_byte) {
                    return userver::utils::unexpected{current_byte.error()};
                }

                for (std::uint8_t bit = 0; bit < 8 && bits_read < expected_quantity; ++bit, ++bits_read) {
                    const bool is_set = (*current_byte & (1U << bit)) != 0;
                    if constexpr (std::is_same_v<T, Coil>) {
                        values.push_back(is_set ? Coil::kOn : Coil::kOff);
                    } else {
                        values.push_back(is_set);
                    }
                }
            }
        } else {
            if (*byte_count % sizeof(std::uint16_t) != 0) {
                return userver::utils::unexpected{ParseError::kInvalidValue};
            }

            const auto register_count = *byte_count / sizeof(std::uint16_t);
            if (register_count < kMinQuantity || register_count > kMaxQuantity) {
                return userver::utils::unexpected{ParseError::kInvalidQuantity};
            }

            values.reserve(register_count);

            for (std::size_t i = 0; i < register_count; ++i) {
                const auto reg = ReadBe<std::uint16_t>(first, last);
                if (!reg) {
                    return userver::utils::unexpected{reg.error()};
                }
                values.push_back(*reg);
            }
        }

        if (first != last) {
            return userver::utils::unexpected{ParseError::kExtraDataAtEnd};
        }

        return ReadMultiple{std::move(values)};
    }

    template <typename OutputIt>
    [[nodiscard]] OutputIt Serialize(OutputIt out
    ) const noexcept(noexcept(WriteBe(out, static_cast<std::uint8_t>(kFunctionCode)))) {
        out = WriteBe(out, static_cast<std::uint8_t>(kFunctionCode));
        out = WriteBe(out, GetByteCount());

        if constexpr (std::is_same_v<T, Coil> || std::is_same_v<T, bool>) {
            std::uint8_t current_byte = 0;
            std::uint8_t bit_index = 0;

            for (const auto& val : values_) {
                bool is_set = false;
                if constexpr (std::is_same_v<T, Coil>) {
                    is_set = (val == Coil::kOn);
                } else {
                    is_set = static_cast<bool>(val);
                }

                if (is_set) {
                    current_byte |= static_cast<std::uint8_t>(1U << bit_index);
                }

                ++bit_index;
                if (bit_index == 8) {
                    out = WriteBe(out, current_byte);
                    current_byte = 0;
                    bit_index = 0;
                }
            }

            if (bit_index > 0) {
                out = WriteBe(out, current_byte);
            }
        } else {
            for (const auto reg : values_) {
                out = WriteBe(out, reg);
            }
        }

        return out;
    }

    [[nodiscard]] std::uint8_t GetByteCount() const noexcept {
        if constexpr (std::is_same_v<T, Coil> || std::is_same_v<T, bool>) {
            return static_cast<std::uint8_t>(BitsToBytes(values_.size()));
        } else {
            return static_cast<std::uint8_t>(values_.size() * sizeof(T));
        }
    }

    [[nodiscard]] std::span<const T> GetValues() const noexcept { return values_; }

    [[nodiscard]] std::span<const T> GetRegisters() const noexcept
    requires std::is_same_v<T, std::uint16_t>
    {
        return values_;
    }

private:
    explicit ReadMultiple(std::vector<T> values) noexcept : values_{std::move(values)} {}

    std::vector<T> values_;
};

}  // namespace modbus::response::impl
