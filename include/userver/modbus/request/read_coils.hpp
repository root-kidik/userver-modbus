#pragma once

#include <userver/modbus/function_code.hpp>
#include <userver/modbus/utils.hpp>

#include <userver/utils/expected.hpp>

namespace modbus::request {

class ReadCoils {
public:
  static constexpr std::uint16_t kMinQuantity{0x00'01};
  static constexpr std::uint16_t kMaxQuantity{0x07'D0};

  static userver::utils::expected<ReadCoils, Error>
  Create(std::uint16_t address, std::uint16_t quantity) noexcept {
    if (quantity < kMinQuantity || quantity > kMaxQuantity) {
      return userver::utils::unexpected{Error::kInvalidQuantity};
    }

    if (WouldAddressOverflow(address, quantity)) {
      return userver::utils::unexpected{Error::kAddressOverflow};
    }

    return ReadCoils{address, quantity};
  }

  template <typename InputIt>
  static userver::utils::expected<ReadCoils, Error>
  Deserialize(InputIt first, InputIt last) noexcept {
    const auto function_code = ReadBe<std::uint8_t>(first, last);
    if (!function_code) {
      return userver::utils::unexpected{function_code.error()};
    }

    if (static_cast<FunctionCode>(*function_code) != FunctionCode::kReadCoils) {
      return userver::utils::unexpected{Error::kInvalidFunctionCode};
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
      return userver::utils::unexpected{Error::kExtraDataAtEnd};
    }

    return Create(*address, *quantity);
  }

  template <typename OutputIt> OutputIt Serialize(OutputIt out) const {
    out = WriteBe(out, static_cast<std::uint8_t>(FunctionCode::kReadCoils));
    out = WriteBe(out, address_);
    out = WriteBe(out, quantity_);
    return out;
  }

  [[nodiscard]] std::uint16_t GetAddress() const noexcept { return address_; }

  [[nodiscard]] std::uint16_t GetQuantity() const noexcept { return quantity_; }

private:
  ReadCoils(std::uint16_t address, std::uint16_t quantity)
      : address_{address}, quantity_{quantity} {}

  std::uint16_t address_;
  std::uint16_t quantity_;
};

} // namespace modbus::request
