#pragma once

#include <modbus/request/impl/write_multiple.hpp>

namespace modbus::request {

using WriteMultipleCoils = impl::WriteMultiple<Coil, FunctionCode::kWriteMultipleCoils, 1968>;

}  // namespace modbus::request
