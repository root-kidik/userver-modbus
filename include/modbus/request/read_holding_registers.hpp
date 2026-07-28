#pragma once

#include <modbus/request/impl/read_multiple.hpp>

namespace modbus::request {

using ReadHoldingRegisters = impl::ReadMultiple<FunctionCode::kReadHoldingRegisters, 125>;

}  // namespace modbus::request
