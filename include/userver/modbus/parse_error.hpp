#pragma once

namespace modbus {

enum class ParseError {
    kBufferTooShort,
    kExtraDataAtEnd,
    kInvalidFunctionCode,
    kInvalidExceptionCode,
    kInvalidQuantity,
    kAddressOverflow,
    kInvalidValue,
};

}  // namespace modbus
