#pragma once

#include <modbus/request/impl/write_single.hpp>

namespace modbus::request {

using WriteSingleHoldingRegister = impl::WriteSingle<std::uint16_t, FunctionCode::kWriteSingleHoldingRegister>;

}  // namespace modbus::request
