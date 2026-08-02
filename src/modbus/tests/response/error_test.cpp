#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/response/error.hpp>

UTEST(ResponseErrorTest, CreateAndGetters) {
    const auto error = modbus::response::ErrorResponse::Create(
        modbus::FunctionCode::kReadCoils,
        modbus::ExceptionCode::kIllegalDataAddress
    );

    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(error->GetFunctionCode(), modbus::FunctionCode::kReadCoils);
    EXPECT_EQ(error->GetExceptionCode(), modbus::ExceptionCode::kIllegalDataAddress);
}

UTEST(ResponseErrorTest, Serialize) {
    const auto error = modbus::response::ErrorResponse::Create(
        modbus::FunctionCode::kReadCoils,
        modbus::ExceptionCode::kIllegalDataAddress
    );
    ASSERT_TRUE(error.has_value());

    std::vector<std::uint8_t> bytes;
    std::ignore = error->Serialize(std::back_inserter(bytes));

    const std::vector<std::uint8_t> expected_bytes{
        modbus::ToErrorFunctionCode(modbus::FunctionCode::kReadCoils),
        static_cast<std::uint8_t>(modbus::ExceptionCode::kIllegalDataAddress)
    };
    EXPECT_EQ(bytes, expected_bytes);
}

UTEST(ResponseErrorTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{
        modbus::ToErrorFunctionCode(modbus::FunctionCode::kReadCoils),
        static_cast<std::uint8_t>(modbus::ExceptionCode::kIllegalDataAddress)
    };
    auto it = buffer.cbegin();

    const auto deserialized = modbus::response::ErrorResponse::Deserialize(it, buffer.cend());
    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized->GetFunctionCode(), modbus::FunctionCode::kReadCoils);
    EXPECT_EQ(deserialized->GetExceptionCode(), modbus::ExceptionCode::kIllegalDataAddress);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(ResponseErrorTest, DeserializeBufferTooShortRawFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto result = modbus::response::ErrorResponse::Deserialize(it, buffer.cend());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseErrorTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x01, 0x02};
    auto it = buffer.cbegin();

    const auto result = modbus::response::ErrorResponse::Deserialize(it, buffer.cend());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseErrorTest, DeserializeBufferTooShortRawEc) {
    const std::vector<std::uint8_t> buffer{0x81};
    auto it = buffer.cbegin();

    const auto result = modbus::response::ErrorResponse::Deserialize(it, buffer.cend());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseErrorTest, DeserializeInvalidExceptionCode) {
    const std::vector<std::uint8_t> buffer{0x81, 0xFF};
    auto it = buffer.cbegin();

    const auto result = modbus::response::ErrorResponse::Deserialize(it, buffer.cend());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kInvalidExceptionCode);
}

UTEST(ResponseErrorTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x81, 0x02, 0x00};
    auto it = buffer.cbegin();

    const auto result = modbus::response::ErrorResponse::Deserialize(it, buffer.cend());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kExtraDataAtEnd);
}
