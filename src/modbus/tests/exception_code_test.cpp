#include <userver/utest/utest.hpp>

#include <modbus/exception_code.hpp>

UTEST(ExceptionCodeTest, IsValidExceptionCodeEnum) {
    EXPECT_TRUE(modbus::IsValidExceptionCode(modbus::ExceptionCode::kIllegalFunction));
    EXPECT_TRUE(modbus::IsValidExceptionCode(modbus::ExceptionCode::kIllegalDataAddress));
    EXPECT_TRUE(modbus::IsValidExceptionCode(modbus::ExceptionCode::kIllegalDataValue));
    EXPECT_TRUE(modbus::IsValidExceptionCode(modbus::ExceptionCode::kServerDeviceFailure));
    EXPECT_TRUE(modbus::IsValidExceptionCode(modbus::ExceptionCode::kAcknowledge));
    EXPECT_TRUE(modbus::IsValidExceptionCode(modbus::ExceptionCode::kServerDeviceBusy));
    EXPECT_TRUE(modbus::IsValidExceptionCode(modbus::ExceptionCode::kMemoryParityError));
    EXPECT_TRUE(modbus::IsValidExceptionCode(modbus::ExceptionCode::kGatewayPathUnavailable));
    EXPECT_TRUE(modbus::IsValidExceptionCode(modbus::ExceptionCode::kGatewayTargetFailed));
}

UTEST(ExceptionCodeTest, IsValidExceptionCodeRaw) {
    EXPECT_TRUE(modbus::IsValidExceptionCode(static_cast<std::uint8_t>(modbus::ExceptionCode::kIllegalFunction)));
    EXPECT_TRUE(modbus::IsValidExceptionCode(static_cast<std::uint8_t>(modbus::ExceptionCode::kIllegalDataAddress)));
    EXPECT_TRUE(modbus::IsValidExceptionCode(static_cast<std::uint8_t>(modbus::ExceptionCode::kIllegalDataValue)));
    EXPECT_TRUE(modbus::IsValidExceptionCode(static_cast<std::uint8_t>(modbus::ExceptionCode::kServerDeviceFailure)));
    EXPECT_TRUE(modbus::IsValidExceptionCode(static_cast<std::uint8_t>(modbus::ExceptionCode::kAcknowledge)));
    EXPECT_TRUE(modbus::IsValidExceptionCode(static_cast<std::uint8_t>(modbus::ExceptionCode::kServerDeviceBusy)));
    EXPECT_TRUE(modbus::IsValidExceptionCode(static_cast<std::uint8_t>(modbus::ExceptionCode::kMemoryParityError)));
    EXPECT_TRUE(modbus::IsValidExceptionCode(static_cast<std::uint8_t>(modbus::ExceptionCode::kGatewayPathUnavailable))
    );
    EXPECT_TRUE(modbus::IsValidExceptionCode(static_cast<std::uint8_t>(modbus::ExceptionCode::kGatewayTargetFailed)));

    EXPECT_FALSE(modbus::IsValidExceptionCode(0x0));
}
