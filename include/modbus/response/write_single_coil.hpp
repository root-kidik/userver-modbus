#pragma once

#include <modbus/coil.hpp>
#include <modbus/response/impl/write_single.hpp>

namespace modbus::response {

using WriteSingleCoil = impl::WriteSingle<Coil, FunctionCode::kWriteSingleCoil>;

}  // namespace modbus::response
