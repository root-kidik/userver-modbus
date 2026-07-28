#pragma once

#include <cstdint>
#include <modbus/response/impl/write_single.hpp>

namespace modbus::response {

using WriteSingleHoldingRegister = impl::WriteSingle<std::uint16_t, FunctionCode::kWriteSingleHoldingRegister>;

}  // namespace modbus::response
