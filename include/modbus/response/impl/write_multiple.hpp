#pragma once

#include <cstdint>

#include <modbus/function_code.hpp>
#include <modbus/parse_error.hpp>
#include <modbus/utils.hpp>

#include <userver/utils/expected.hpp>

namespace modbus::response::impl {

template <FunctionCode Code, std::uint16_t MaxQty, std::uint16_t MinQty = 1>
class WriteMultiple {
public:
    static constexpr std::uint16_t kMinQuantity{MinQty};
    static constexpr std::uint16_t kMaxQuantity{MaxQty};

    static constexpr FunctionCode kFunctionCode{Code};

    static userver::utils::expected<WriteMultiple, ParseError> Create(std::uint16_t address, std::uint16_t quantity)
        noexcept {
        if (quantity < kMinQuantity || quantity > kMaxQuantity) {
            return userver::utils::unexpected{ParseError::kInvalidQuantity};
        }

        if (WouldAddressOverflow(address, quantity)) {
            return userver::utils::unexpected{ParseError::kAddressOverflow};
        }

        return WriteMultiple{address, quantity};
    }

    template <typename InputIt>
    static userver::utils::expected<WriteMultiple, ParseError> Deserialize(InputIt& first, InputIt last) noexcept {
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

        if (first != last) {
            return userver::utils::unexpected{ParseError::kExtraDataAtEnd};
        }

        return Create(*address, *quantity);
    }

    template <typename OutputIt>
    [[nodiscard]] OutputIt Serialize(OutputIt out
    ) const noexcept(noexcept(WriteBe(out, static_cast<std::uint8_t>(kFunctionCode)))) {
        out = WriteBe(out, static_cast<std::uint8_t>(kFunctionCode));
        out = WriteBe(out, address_);
        out = WriteBe(out, quantity_);
        return out;
    }

    [[nodiscard]] std::uint16_t GetAddress() const noexcept { return address_; }

    [[nodiscard]] std::uint16_t GetQuantity() const noexcept { return quantity_; }

private:
    constexpr WriteMultiple(std::uint16_t address, std::uint16_t quantity) noexcept
        : address_{address}, quantity_{quantity} {}

    std::uint16_t address_;
    std::uint16_t quantity_;
};

}  // namespace modbus::response::impl
