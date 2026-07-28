#pragma once

#include <modbus/request/impl/read_multiple.hpp>

namespace modbus::request {

using ReadCoils = impl::ReadMultiple<FunctionCode::kReadCoils, 2000>;

}  // namespace modbus::request
