#pragma once

#include <userver/modbus/coil.hpp>
#include <userver/modbus/response/impl/write_single.hpp>

namespace modbus::response {

using WriteSingleCoil = impl::WriteSingle<Coil, FunctionCode::kWriteSingleCoil>;

}  // namespace modbus::response
