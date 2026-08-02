#pragma once

#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include <modbus/coil.hpp>
#include <modbus/function_code.hpp>
#include <modbus/utils.hpp>

#include <userver/utils/expected.hpp>

namespace modbus::request::impl {

template <typename T, FunctionCode Code, std::uint16_t MaxQty, std::uint16_t MinQty = 1>
class WriteMultiple {
public:
    static constexpr std::uint16_t kMinQuantity{MinQty};
    static constexpr std::uint16_t kMaxQuantity{MaxQty};

    static constexpr FunctionCode kFunctionCode{Code};

    static userver::utils::expected<WriteMultiple, ParseError> Create(std::uint16_t address, std::vector<T> values) {
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

        return WriteMultiple{address, std::move(values)};
    }

    template <typename InputIt>
    static userver::utils::expected<WriteMultiple, ParseError> Deserialize(InputIt& first, InputIt last) {
        const auto function_code = ReadBe<std::uint8_t>(first, last);
        if (!function_code) {
            return userver::utils::unexpected{function_code.error()};
        }

        if (static_cast<FunctionCode>(*function_code) != kFunctionCode) {
            return userver::utils::unexpected{ParseError::kInvalidFunctionCode};
        }

        const auto address = ReadBe<std::uint16_t>(first, last);
        if (!address) {
            return userver::utils::unexpected{address.error()};
        }

        const auto quantity = ReadBe<std::uint16_t>(first, last);
        if (!quantity) {
            return userver::utils::unexpected{quantity.error()};
        }

        if (*quantity < kMinQuantity || *quantity > kMaxQuantity) {
            return userver::utils::unexpected{ParseError::kInvalidQuantity};
        }

        if (WouldAddressOverflow(*address, *quantity)) {
            return userver::utils::unexpected{ParseError::kAddressOverflow};
        }

        const auto byte_count = ReadBe<std::uint8_t>(first, last);
        if (!byte_count) {
            return userver::utils::unexpected{byte_count.error()};
        }

        const std::uint8_t expected_byte_count = CalcByteCount(*quantity);
        if (*byte_count != expected_byte_count) {
            return userver::utils::unexpected{ParseError::kInvalidQuantity};
        }

        std::vector<T> values;
        values.reserve(*quantity);

        if constexpr (std::is_same_v<T, Coil>) {
            std::uint8_t current_byte = 0;
            for (std::size_t i = 0; i < *quantity; ++i) {
                const auto bit_idx = i % 8;
                if (bit_idx == 0) {
                    const auto byte_val = ReadBe<std::uint8_t>(first, last);
                    if (!byte_val) {
                        return userver::utils::unexpected{byte_val.error()};
                    }
                    current_byte = *byte_val;
                }

                const auto is_on = ((current_byte >> bit_idx) & 0x01) != 0;
                values.push_back(is_on ? Coil::kOn : Coil::kOff);
            }
        } else {
            for (std::size_t i = 0; i < *quantity; ++i) {
                const auto val = ReadBe<T>(first, last);
                if (!val) {
                    return userver::utils::unexpected{val.error()};
                }
                values.push_back(*val);
            }
        }

        if (first != last) {
            return userver::utils::unexpected{ParseError::kExtraDataAtEnd};
        }

        return WriteMultiple{*address, std::move(values)};
    }

    template <typename OutputIt>
    [[nodiscard]] OutputIt Serialize(OutputIt out
    ) const noexcept(noexcept(WriteBe(out, static_cast<std::uint8_t>(kFunctionCode)))) {
        out = WriteBe(out, static_cast<std::uint8_t>(kFunctionCode));
        out = WriteBe(out, address_);
        out = WriteBe(out, GetQuantity());
        out = WriteBe(out, GetByteCount());

        if constexpr (std::is_same_v<T, Coil>) {
            const auto byte_count = GetByteCount();
            for (std::size_t byte_idx = 0; byte_idx < byte_count; ++byte_idx) {
                std::uint8_t byte_val = 0;
                for (std::size_t bit_idx = 0; bit_idx < 8; ++bit_idx) {
                    const auto coil_idx = byte_idx * 8 + bit_idx;
                    if (coil_idx < values_.size() && values_[coil_idx] == Coil::kOn) {
                        byte_val |= static_cast<std::uint8_t>(1U << bit_idx);
                    }
                }
                out = WriteBe(out, byte_val);
            }
        } else {
            for (const auto val : values_) {
                out = WriteBe(out, val);
            }
        }

        return out;
    }

    [[nodiscard]] std::uint16_t GetAddress() const noexcept { return address_; }

    [[nodiscard]] std::uint16_t GetQuantity() const noexcept { return static_cast<std::uint16_t>(values_.size()); }

    [[nodiscard]] std::uint8_t GetByteCount() const noexcept { return CalcByteCount(GetQuantity()); }

    [[nodiscard]] std::span<const T> GetValues() const noexcept { return values_; }

    [[nodiscard]] std::span<const T> GetCoils() const noexcept
    requires std::is_same_v<T, Coil>
    {
        return values_;
    }

    [[nodiscard]] std::span<const T> GetRegisters() const noexcept
    requires std::is_same_v<T, std::uint16_t>
    {
        return values_;
    }

private:
    WriteMultiple(std::uint16_t address, std::vector<T> values) noexcept
        : address_{address}, values_{std::move(values)} {}

    static constexpr std::uint8_t CalcByteCount(std::size_t quantity) noexcept {
        if constexpr (std::is_same_v<T, Coil>) {
            return static_cast<std::uint8_t>((quantity + 7) / 8);
        } else {
            return static_cast<std::uint8_t>(quantity * sizeof(T));
        }
    }

    std::uint16_t address_;
    std::vector<T> values_;
};

}  // namespace modbus::request::impl
