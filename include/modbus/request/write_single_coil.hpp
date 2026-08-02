#pragma once

#include <modbus/request/impl/write_single.hpp>

namespace modbus::request {

using WriteSingleCoil = impl::WriteSingle<Coil, FunctionCode::kWriteSingleCoil>;

}  // namespace modbus::request
