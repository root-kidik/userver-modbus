#include <array>
#include <cstddef>
#include <span>

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

    std::array<std::byte, modbus::response::ErrorResponse::kEncodedSize> bytes{};
    const auto result = error->Serialize(bytes);
    ASSERT_TRUE(result.has_value());

    const std::array<std::byte, 2> expected_bytes{
        static_cast<std::byte>(modbus::ToErrorFunctionCode(modbus::FunctionCode::kReadCoils)),
        static_cast<std::byte>(modbus::ExceptionCode::kIllegalDataAddress)
    };
    EXPECT_EQ(bytes, expected_bytes);
}

UTEST(ResponseErrorTest, DeserializeSuccess) {
    const std::array<std::byte, 2> buffer{
        static_cast<std::byte>(modbus::ToErrorFunctionCode(modbus::FunctionCode::kReadCoils)),
        static_cast<std::byte>(modbus::ExceptionCode::kIllegalDataAddress)
    };
    std::span<const std::byte> span = buffer;

    const auto deserialized = modbus::response::ErrorResponse::Deserialize(span);
    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized->GetFunctionCode(), modbus::FunctionCode::kReadCoils);
    EXPECT_EQ(deserialized->GetExceptionCode(), modbus::ExceptionCode::kIllegalDataAddress);
    EXPECT_TRUE(span.empty());
}

UTEST(ResponseErrorTest, DeserializeBufferTooShortRawFc) {
    const std::array<std::byte, 0> buffer{};
    std::span<const std::byte> span = buffer;

    const auto result = modbus::response::ErrorResponse::Deserialize(span);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseErrorTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 2> buffer{std::byte{0x01}, std::byte{0x02}};
    std::span<const std::byte> span = buffer;

    const auto result = modbus::response::ErrorResponse::Deserialize(span);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseErrorTest, DeserializeBufferTooShortRawEc) {
    const std::array<std::byte, 1> buffer{std::byte{0x81}};
    std::span<const std::byte> span = buffer;

    const auto result = modbus::response::ErrorResponse::Deserialize(span);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseErrorTest, DeserializeInvalidExceptionCode) {
    const std::array<std::byte, 2> buffer{std::byte{0x81}, std::byte{0xFF}};
    std::span<const std::byte> span = buffer;

    const auto result = modbus::response::ErrorResponse::Deserialize(span);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kInvalidExceptionCode);
}
