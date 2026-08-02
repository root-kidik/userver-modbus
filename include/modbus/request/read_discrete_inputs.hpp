#pragma once

#include <modbus/request/impl/read_multiple.hpp>

namespace modbus::request {

using ReadDiscreteInputs = impl::ReadMultiple<FunctionCode::kReadDiscreteInputs, 2000>;

}  // namespace modbus::request
