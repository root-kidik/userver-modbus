#include <array>
#include <cstddef>
#include <span>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/request/write_single_holding_register.hpp>

UTEST(RequestWriteSingleHoldingRegisterTest, CreateSuccess) {
    const auto request = modbus::request::WriteSingleHoldingRegister::Create(0x0001, 0x0003);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0001);
    EXPECT_EQ(request->GetValue(), 0x0003);
}

UTEST(RequestWriteSingleHoldingRegisterTest, SerializeSuccess) {
    const auto request = modbus::request::WriteSingleHoldingRegister::Create(0x0001, 0x0003);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x06}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x03}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(RequestWriteSingleHoldingRegisterTest, SerializeBufferTooShort) {
    const auto request = modbus::request::WriteSingleHoldingRegister::Create(0x0001, 0x0003);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializeSuccess) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x06}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x03}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0001);
    EXPECT_EQ(request->GetValue(), 0x0003);
    EXPECT_TRUE(buffer.empty());
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x06}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializeBufferTooShortValue) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x06}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x05}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x03}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x06},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x03},
        std::byte{0x00}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0x00});
}
