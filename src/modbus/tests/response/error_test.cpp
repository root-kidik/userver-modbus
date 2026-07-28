#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/response/error.hpp>

UTEST(ResponseErrorTest, Error) {
    const auto error = modbus::response::ErrorResponse::Create(
        modbus::FunctionCode::kReadCoils,
        modbus::ExceptionCode::kIllegalDataAddress
    );

    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(error->GetFunctionCode(), modbus::FunctionCode::kReadCoils);
    EXPECT_EQ(error->GetExceptionCode(), modbus::ExceptionCode::kIllegalDataAddress);

    std::vector<std::uint8_t> bytes;
    std::ignore = error->Serialize(std::back_inserter(bytes));

    const std::vector<std::uint8_t> expected_bytes{
        modbus::ToErrorFunctionCode(modbus::FunctionCode::kReadCoils),
        static_cast<std::uint8_t>(modbus::ExceptionCode::kIllegalDataAddress)
    };
    EXPECT_EQ(bytes, expected_bytes);

    auto it = expected_bytes.cbegin();
    const auto deserialized = modbus::response::ErrorResponse::Deserialize(it, expected_bytes.cend());

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized->GetFunctionCode(), modbus::FunctionCode::kReadCoils);
    EXPECT_EQ(deserialized->GetExceptionCode(), modbus::ExceptionCode::kIllegalDataAddress);
}
