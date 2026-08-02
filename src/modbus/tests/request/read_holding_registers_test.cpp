#include <array>
#include <cstddef>
#include <span>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/request/read_holding_registers.hpp>

UTEST(RequestReadHoldingRegistersTest, CreateSuccess) {
    const auto request = modbus::request::ReadHoldingRegisters::Create(0x006B, 3);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x006B);
    EXPECT_EQ(request->GetQuantity(), 3);
}

UTEST(RequestReadHoldingRegistersTest, CreateMinQuantity) {
    const auto request =
        modbus::request::ReadHoldingRegisters::Create(0x0000, modbus::request::ReadHoldingRegisters::kMinQuantity);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestReadHoldingRegistersTest, CreateMaxQuantity) {
    const auto request =
        modbus::request::ReadHoldingRegisters::Create(0x0000, modbus::request::ReadHoldingRegisters::kMaxQuantity);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 125);
}

UTEST(RequestReadHoldingRegistersTest, CreateInvalidQuantityUnderflow) {
    const auto request = modbus::request::ReadHoldingRegisters::Create(0x006B, 0);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadHoldingRegistersTest, CreateInvalidQuantityOverflow) {
    const auto request = modbus::request::ReadHoldingRegisters::Create(0x006B, 126);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadHoldingRegistersTest, CreateAddressOverflow) {
    const auto request = modbus::request::ReadHoldingRegisters::Create(0xFFFF, 2);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestReadHoldingRegistersTest, SerializeSuccess) {
    const auto request = modbus::request::ReadHoldingRegisters::Create(0x006B, 0x0003);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x03}, std::byte{0x00}, std::byte{0x6B}, std::byte{0x00}, std::byte{0x03}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(RequestReadHoldingRegistersTest, SerializeBufferTooShort) {
    const auto request = modbus::request::ReadHoldingRegisters::Create(0x006B, 0x0003);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeSuccess) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x03}, std::byte{0x00}, std::byte{0x6B}, std::byte{0x00}, std::byte{0x03}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x006B);
    EXPECT_EQ(request->GetQuantity(), 3);
    EXPECT_TRUE(buffer.empty());
}

UTEST(RequestReadHoldingRegistersTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x03}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeBufferTooShortQuantity) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x03}, std::byte{0x00}, std::byte{0x6B}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x04}, std::byte{0x00}, std::byte{0x6B}, std::byte{0x00}, std::byte{0x03}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestReadHoldingRegistersTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x03},
        std::byte{0x00},
        std::byte{0x6B},
        std::byte{0x00},
        std::byte{0x03},
        std::byte{0x01}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0x01});
}

UTEST(RequestReadHoldingRegistersTest, DeserializeInvalidQuantity) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x03}, std::byte{0x00}, std::byte{0x6B}, std::byte{0x00}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeAddressOverflow) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x03}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0x00}, std::byte{0x02}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}
