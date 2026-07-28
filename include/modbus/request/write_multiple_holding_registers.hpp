#pragma once

#include <modbus/request/impl/write_multiple.hpp>

namespace modbus::request {

using WriteMultipleHoldingRegisters =
    impl::WriteMultiple<std::uint16_t, FunctionCode::kWriteMultipleHoldingRegisters, 123>;

}  // namespace modbus::request
