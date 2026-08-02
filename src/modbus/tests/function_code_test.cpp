#include <userver/utest/utest.hpp>

#include <modbus/function_code.hpp>

UTEST(FunctionCodeTest, ToErrorFunctionCode) {
    const auto raw_function_code = static_cast<std::uint8_t>(modbus::FunctionCode::kReadCoils);
    const auto error_function_code = modbus::ToErrorFunctionCode(raw_function_code);

    EXPECT_EQ(error_function_code, 0x81);
}

UTEST(FunctionCodeTest, IsErrorFunctionCode) {
    EXPECT_TRUE(modbus::IsErrorFunctionCode(
        modbus::ToErrorFunctionCode(static_cast<std::uint8_t>(modbus::FunctionCode::kReadCoils))
    ));

    EXPECT_TRUE(modbus::IsErrorFunctionCode(modbus::ToErrorFunctionCode(modbus::FunctionCode::kReadCoils)));
}

UTEST(FunctionCodeTest, ToNormalFunctionCode) {
    const auto raw_function_code = static_cast<std::uint8_t>(modbus::FunctionCode::kReadCoils);

    EXPECT_EQ(modbus::ToNormalFunctionCode(modbus::ToErrorFunctionCode(raw_function_code)), raw_function_code);
}
