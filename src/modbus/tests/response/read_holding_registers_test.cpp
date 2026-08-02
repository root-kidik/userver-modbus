#include <array>
#include <cstddef>
#include <span>
#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/response/read_holding_registers.hpp>

UTEST(ResponseReadHoldingRegistersTest, CreateSuccess) {
    const std::array<std::uint16_t, 2> registers{0x022B, 0x0000};
    const auto response = modbus::response::ReadHoldingRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 4);
    EXPECT_EQ(response->GetValues().size(), 2);
}

UTEST(ResponseReadHoldingRegistersTest, CreateMinQuantity) {
    const std::array<std::uint16_t, 1> registers{0x0001};
    const auto response = modbus::response::ReadHoldingRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetValues().size(), 1);
}

UTEST(ResponseReadHoldingRegistersTest, CreateMaxQuantity) {
    const std::vector<std::uint16_t> registers(125, 0x0001);
    const auto response = modbus::response::ReadHoldingRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetValues().size(), 125);
}

UTEST(ResponseReadHoldingRegistersTest, CreateInvalidQuantityZero) {
    const std::array<std::uint16_t, 0> registers{};
    const auto response = modbus::response::ReadHoldingRegisters::Create(registers);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadHoldingRegistersTest, CreateInvalidQuantityOverflow) {
    const std::vector<std::uint16_t> registers(126, 0x0001);
    const auto response = modbus::response::ReadHoldingRegisters::Create(registers);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadHoldingRegistersTest, SerializeSuccess) {
    const std::array<std::uint16_t, 2> registers{0x022B, 0x0000};
    const auto response = modbus::response::ReadHoldingRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 6> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 6>
        expected{std::byte{0x03}, std::byte{0x04}, std::byte{0x02}, std::byte{0x2B}, std::byte{0x00}, std::byte{0x00}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(ResponseReadHoldingRegistersTest, SerializeBufferTooShort) {
    const std::array<std::uint16_t, 2> registers{0x022B, 0x0000};
    const auto response = modbus::response::ReadHoldingRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeSuccess) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x02},
        std::byte{0x2B},
        std::byte{0x00},
        std::byte{0x00}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(buffer, 2);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 4);

    const auto registers = response->GetValues();
    ASSERT_EQ(registers.size(), 2);
    EXPECT_EQ(registers[0], 0x022B);
    EXPECT_EQ(registers[1], 0x0000);
    EXPECT_TRUE(buffer.empty());
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeBufferTooShortByteCount) {
    const std::array<std::byte, 1> raw_buffer{std::byte{0x03}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeBufferTooShortData) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x03}, std::byte{0x04}, std::byte{0x02}, std::byte{0x2B}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(buffer, 2);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x04}, std::byte{0x02}, std::byte{0x12}, std::byte{0x34}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeInvalidByteCountOdd) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x03}, std::byte{0x03}, std::byte{0x12}, std::byte{0x34}, std::byte{0x56}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeInvalidQuantityZero) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x03}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(buffer, 0);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeInvalidQuantityOverflow) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x03}, std::byte{0xFC}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(buffer, 126);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x03}, std::byte{0x02}, std::byte{0x00}, std::byte{0x0A}, std::byte{0xEE}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(buffer, 1);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0xEE});
}
