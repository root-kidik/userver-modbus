#pragma once

#include <modbus/coil.hpp>
#include <modbus/response/impl/read_multiple.hpp>

namespace modbus::response {

using ReadCoils = impl::ReadMultiple<Coil, FunctionCode::kReadCoils, 2000>;

}  // namespace modbus::response
