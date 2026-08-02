#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include <modbus/function_code.hpp>
#include <modbus/parse_error.hpp>
#include <modbus/utils.hpp>

#include <userver/utils/expected.hpp>

namespace modbus::request::impl {

template <FunctionCode Code, std::uint16_t MaxQty, std::uint16_t MinQty = 1>
class ReadMultiple {
public:
    static constexpr std::uint16_t kMinQuantity{MinQty};
    static constexpr std::uint16_t kMaxQuantity{MaxQty};

    static constexpr FunctionCode kFunctionCode{Code};

    static constexpr std::size_t kEncodedSize = 5;

    static userver::utils::expected<ReadMultiple, ParseError> Create(std::uint16_t address, std::uint16_t quantity)
        noexcept {
        if (quantity < kMinQuantity || quantity > kMaxQuantity) {
            return userver::utils::unexpected{ParseError::kInvalidQuantity};
        }

        if (WouldAddressOverflow(address, quantity)) {
            return userver::utils::unexpected{ParseError::kAddressOverflow};
        }

        return ReadMultiple{address, quantity};
    }

    static userver::utils::expected<ReadMultiple, ParseError> Deserialize(std::span<const std::byte>& buffer) noexcept {
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

        return Create(*address, *quantity);
    }

    userver::utils::expected<std::span<std::byte>, ParseError> Serialize(std::span<std::byte> out) const noexcept {
        if (out.size() < kEncodedSize) {
            return userver::utils::unexpected{ParseError::kBufferTooShort};
        }

        std::ignore = WriteBe(out, static_cast<std::uint8_t>(kFunctionCode));
        std::ignore = WriteBe(out, address_);
        std::ignore = WriteBe(out, quantity_);

        return out;
    }

    [[nodiscard]] std::uint16_t GetAddress() const noexcept { return address_; }

    [[nodiscard]] std::uint16_t GetQuantity() const noexcept { return quantity_; }

private:
    constexpr ReadMultiple(std::uint16_t address, std::uint16_t quantity) noexcept
        : address_{address}, quantity_{quantity} {}

    std::uint16_t address_;
    std::uint16_t quantity_;
};

}  // namespace modbus::request::impl
