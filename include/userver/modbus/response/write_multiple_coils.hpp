#pragma once

#include <userver/modbus/response/impl/write_multiple.hpp>

namespace modbus::response {

using WriteMultipleCoils = impl::WriteMultiple<FunctionCode::kWriteMultipleCoils, 1968>;

}  // namespace modbus::response
