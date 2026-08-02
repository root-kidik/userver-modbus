#include <array>
#include <cstddef>
#include <span>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/response/write_single_holding_register.hpp>

UTEST(ResponseWriteSingleHoldingRegisterTest, CreateSuccess) {
    const auto response = modbus::response::WriteSingleHoldingRegister::Create(0x0001, 0x0003);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x0001);
    EXPECT_EQ(response->GetValue(), 0x0003);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, SerializeSuccess) {
    const auto response = modbus::response::WriteSingleHoldingRegister::Create(0x0001, 0x0003);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x06}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x03}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, SerializeBufferTooShort) {
    const auto response = modbus::response::WriteSingleHoldingRegister::Create(0x0001, 0x0003);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializeSuccess) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x06}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x03}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x0001);
    EXPECT_EQ(response->GetValue(), 0x0003);
    EXPECT_TRUE(buffer.empty());
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x06}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializeBufferTooShortValue) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x06}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x05}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x03}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x06},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x03},
        std::byte{0x44}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0x44});
}
