#pragma once

#include <userver/modbus/response/impl/write_multiple.hpp>

namespace modbus::response {

using WriteMultipleHoldingRegisters = impl::WriteMultiple<FunctionCode::kWriteMultipleHoldingRegisters, 123>;

}  // namespace modbus::response
