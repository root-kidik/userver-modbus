#pragma once

#include <cstdint>
#include <type_traits>

#include <modbus/coil.hpp>
#include <modbus/function_code.hpp>
#include <modbus/parse_error.hpp>
#include <modbus/utils.hpp>

#include <userver/utils/expected.hpp>

namespace modbus::response::impl {

template <typename T, FunctionCode Code>
class WriteSingle {
public:
    static constexpr FunctionCode kFunctionCode{Code};

    static userver::utils::expected<WriteSingle, ParseError> Create(std::uint16_t address, T value) noexcept {
        if constexpr (std::is_same_v<T, Coil>) {
            if (value != Coil::kOn && value != Coil::kOff) {
                return userver::utils::unexpected{ParseError::kInvalidValue};
            }
        }

        return WriteSingle{address, value};
    }

    template <typename InputIt>
    static userver::utils::expected<WriteSingle, ParseError> Deserialize(InputIt& first, InputIt last) noexcept {
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

        const auto raw_value = ReadBe<std::uint16_t>(first, last);
        if (!raw_value) {
            return userver::utils::unexpected{raw_value.error()};
        }

        if (first != last) {
            return userver::utils::unexpected{ParseError::kExtraDataAtEnd};
        }

        if constexpr (std::is_same_v<T, Coil>) {
            const auto coil_val = CoilFromRaw(*raw_value);
            if (!coil_val) {
                return userver::utils::unexpected{coil_val.error()};
            }
            return Create(*address, *coil_val);
        } else {
            return Create(*address, *raw_value);
        }
    }

    template <typename OutputIt>
    [[nodiscard]] OutputIt Serialize(OutputIt out
    ) const noexcept(noexcept(WriteBe(out, static_cast<std::uint8_t>(kFunctionCode)))) {
        out = WriteBe(out, static_cast<std::uint8_t>(kFunctionCode));
        out = WriteBe(out, address_);

        if constexpr (std::is_same_v<T, Coil>) {
            out = WriteBe(out, CoilToRaw(value_));
        } else {
            out = WriteBe(out, value_);
        }

        return out;
    }

    [[nodiscard]] std::uint16_t GetAddress() const noexcept { return address_; }

    [[nodiscard]] T GetValue() const noexcept { return value_; }

private:
    constexpr WriteSingle(std::uint16_t address, T value) noexcept : address_{address}, value_{value} {}

    std::uint16_t address_;
    T value_;
};

}  // namespace modbus::response::impl
