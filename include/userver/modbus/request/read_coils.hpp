#pragma once

#include <userver/modbus/function_code.hpp>
#include <userver/modbus/utils.hpp>

#include <userver/utils/expected.hpp>

USERVER_NAMESPACE_BEGIN

namespace modbus::request {

class ReadCoils {
public:
  static constexpr std::uint8_t kSize{5};
  static constexpr std::uint16_t kMinQuantity{0x00'01};
  static constexpr std::uint16_t kMaxQuantity{0x07'D0};

  static utils::expected<ReadCoils, Error> Create(std::uint16_t address,
                                                  std::uint16_t quantity) noexcept {
    if (quantity < kMinQuantity || quantity > kMaxQuantity) {
      return utils::unexpected{Error::kInvalidQuantity};
    }

    if (WouldAddressOverflow(address, quantity)) {
      return utils::unexpected{Error::kAddressOverflow};
    }

    return ReadCoils{address, quantity};
  }

  template <typename InputIt>
  static utils::expected<ReadCoils, Error> Deserialize(InputIt first,
                                                       InputIt last) noexcept {
    const auto func_code_res = ReadBe<std::uint8_t>(first, last);
    if (!func_code_res) {
      return utils::unexpected{func_code_res.error()};
    }

    if (static_cast<FunctionCode>(*func_code_res) != FunctionCode::kReadCoils) {
      return utils::unexpected{Error::kInvalidFunctionCode};
    }

    const auto address_res = ReadBe<std::uint16_t>(first, last);
    if (!address_res) {
      return utils::unexpected{address_res.error()};
    }

    const auto quantity_res = ReadBe<std::uint16_t>(first, last);
    if (!quantity_res) {
      return utils::unexpected{quantity_res.error()};
    }

    if (first != last) {
      return utils::unexpected{Error::kExtraDataAtEnd};
    }

    return Create(*address_res, *quantity_res);
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

USERVER_NAMESPACE_END
