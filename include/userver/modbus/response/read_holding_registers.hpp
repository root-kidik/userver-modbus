#pragma once

#include <userver/modbus/response/impl/read_multiple.hpp>

namespace modbus::response {

using ReadHoldingRegisters = impl::ReadMultiple<std::uint16_t, FunctionCode::kReadHoldingRegisters, 125>;

}  // namespace modbus::response
