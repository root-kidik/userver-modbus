#pragma once

#include <userver/modbus/response/impl/read_multiple.hpp>

namespace modbus::response {

using ReadDiscreteInputs = impl::ReadMultiple<bool, FunctionCode::kReadDiscreteInputs, 2000>;

}  // namespace modbus::response
