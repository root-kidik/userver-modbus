#pragma once

#include <modbus/response/impl/read_multiple.hpp>

namespace modbus::response {

using ReadInputRegisters = impl::ReadMultiple<std::uint16_t, FunctionCode::kReadInputRegisters, 125>;

}  // namespace modbus::response
