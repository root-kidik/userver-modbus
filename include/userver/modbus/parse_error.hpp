#pragma once

namespace modbus {

enum class ParseError {
    kBufferTooShort,
    kExtraDataAtEnd,
    kInvalidFunctionCode,
    kInvalidQuantity,
    kAddressOverflow,
    kInvalidValue,
};

}  // namespace modbus
