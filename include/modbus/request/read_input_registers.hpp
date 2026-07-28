#pragma once

#include <modbus/request/impl/read_multiple.hpp>

namespace modbus::request {

using ReadInputRegisters = impl::ReadMultiple<FunctionCode::kReadInputRegisters, 125>;

}  // namespace modbus::request
