#include <array>
#include <cstddef>
#include <span>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/request/read_input_registers.hpp>

UTEST(RequestReadInputRegistersTest, CreateSuccess) {
    const auto request = modbus::request::ReadInputRegisters::Create(0x0008, 1);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0008);
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestReadInputRegistersTest, CreateMinQuantity) {
    const auto request =
        modbus::request::ReadInputRegisters::Create(0x0000, modbus::request::ReadInputRegisters::kMinQuantity);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestReadInputRegistersTest, CreateMaxQuantity) {
    const auto request =
        modbus::request::ReadInputRegisters::Create(0x0000, modbus::request::ReadInputRegisters::kMaxQuantity);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 125);
}

UTEST(RequestReadInputRegistersTest, CreateInvalidQuantityUnderflow) {
    const auto request = modbus::request::ReadInputRegisters::Create(0x0008, 0);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadInputRegistersTest, CreateInvalidQuantityOverflow) {
    const auto request = modbus::request::ReadInputRegisters::Create(0x0008, 126);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadInputRegistersTest, CreateAddressOverflow) {
    const auto request = modbus::request::ReadInputRegisters::Create(0xFFFF, 2);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestReadInputRegistersTest, SerializeSuccess) {
    const auto request = modbus::request::ReadInputRegisters::Create(0x0008, 0x0001);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x04}, std::byte{0x00}, std::byte{0x08}, std::byte{0x00}, std::byte{0x01}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(RequestReadInputRegistersTest, SerializeBufferTooShort) {
    const auto request = modbus::request::ReadInputRegisters::Create(0x0008, 0x0001);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadInputRegistersTest, DeserializeSuccess) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x04}, std::byte{0x00}, std::byte{0x08}, std::byte{0x00}, std::byte{0x01}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadInputRegisters::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0008);
    EXPECT_EQ(request->GetQuantity(), 1);
    EXPECT_TRUE(buffer.empty());
}

UTEST(RequestReadInputRegistersTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto request = modbus::request::ReadInputRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadInputRegistersTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x04}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadInputRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadInputRegistersTest, DeserializeBufferTooShortQuantity) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x04}, std::byte{0x00}, std::byte{0x08}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadInputRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadInputRegistersTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x03}, std::byte{0x00}, std::byte{0x08}, std::byte{0x00}, std::byte{0x01}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadInputRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestReadInputRegistersTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x04},
        std::byte{0x00},
        std::byte{0x08},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x99}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadInputRegisters::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0x99});
}

UTEST(RequestReadInputRegistersTest, DeserializeInvalidQuantity) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x04}, std::byte{0x00}, std::byte{0x08}, std::byte{0x00}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadInputRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadInputRegistersTest, DeserializeAddressOverflow) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x04}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0x00}, std::byte{0x02}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadInputRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}
