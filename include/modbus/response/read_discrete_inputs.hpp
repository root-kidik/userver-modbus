#pragma once

#include <modbus/discrete_input.hpp>
#include <modbus/response/impl/read_multiple.hpp>

namespace modbus::response {

using ReadDiscreteInputs = impl::ReadMultiple<DiscreteInput, FunctionCode::kReadDiscreteInputs, 2000>;

}  // namespace modbus::response
