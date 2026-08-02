#include <array>
#include <cstddef>
#include <span>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/response/write_multiple_holding_registers.hpp>

UTEST(ResponseWriteMultipleHoldingRegistersTest, CreateSuccess) {
    const auto response = modbus::response::WriteMultipleHoldingRegisters::Create(0x0001, 2);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x0001);
    EXPECT_EQ(response->GetQuantity(), 2);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, CreateMinQuantity) {
    const auto response = modbus::response::WriteMultipleHoldingRegisters::Create(0x0000, 1);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetQuantity(), 1);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, CreateMaxQuantity) {
    const auto response = modbus::response::WriteMultipleHoldingRegisters::Create(0x0000, 123);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetQuantity(), 123);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, CreateInvalidQuantityZero) {
    const auto response = modbus::response::WriteMultipleHoldingRegisters::Create(0x0001, 0);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, CreateInvalidQuantityOverflow) {
    const auto response = modbus::response::WriteMultipleHoldingRegisters::Create(0x0001, 124);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, CreateAddressOverflow) {
    const auto response = modbus::response::WriteMultipleHoldingRegisters::Create(0xFFFF, 2);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, SerializeSuccess) {
    const auto response = modbus::response::WriteMultipleHoldingRegisters::Create(0x0001, 0x0002);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x10}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x02}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, SerializeBufferTooShort) {
    const auto response = modbus::response::WriteMultipleHoldingRegisters::Create(0x0001, 0x0002);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeSuccess) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x10}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x02}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x0001);
    EXPECT_EQ(response->GetQuantity(), 2);
    EXPECT_TRUE(buffer.empty());
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x10}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortQuantity) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x10}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x0F}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x02}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeInvalidQuantity) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x10}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeAddressOverflow) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x10}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0x00}, std::byte{0x02}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x10},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x02},
        std::byte{0xCD}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0xCD});
}
