#pragma once

#include <cstdint>
#include <userver/modbus/response/impl/write_single.hpp>

namespace modbus::response {

using WriteSingleHoldingRegister = impl::WriteSingle<std::uint16_t, FunctionCode::kWriteSingleHoldingRegister>;

}  // namespace modbus::response
