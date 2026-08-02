#include <array>
#include <cstddef>
#include <span>
#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/response/read_input_registers.hpp>

UTEST(ResponseReadInputRegistersTest, CreateSuccess) {
    const std::array<std::uint16_t, 1> registers{0x000A};
    const auto response = modbus::response::ReadInputRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 2);
    EXPECT_EQ(response->GetValues().size(), 1);
}

UTEST(ResponseReadInputRegistersTest, CreateMinQuantity) {
    const std::array<std::uint16_t, 1> registers{0x0001};
    const auto response = modbus::response::ReadInputRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetValues().size(), 1);
}

UTEST(ResponseReadInputRegistersTest, CreateMaxQuantity) {
    const std::vector<std::uint16_t> registers(125, 0x0001);
    const auto response = modbus::response::ReadInputRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetValues().size(), 125);
}

UTEST(ResponseReadInputRegistersTest, CreateInvalidQuantityZero) {
    const std::array<std::uint16_t, 0> registers{};
    const auto response = modbus::response::ReadInputRegisters::Create(registers);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadInputRegistersTest, CreateInvalidQuantityOverflow) {
    const std::vector<std::uint16_t> registers(126, 0x0001);
    const auto response = modbus::response::ReadInputRegisters::Create(registers);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadInputRegistersTest, SerializeSuccess) {
    const std::array<std::uint16_t, 1> registers{0x000A};
    const auto response = modbus::response::ReadInputRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 4> expected{std::byte{0x04}, std::byte{0x02}, std::byte{0x00}, std::byte{0x0A}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(ResponseReadInputRegistersTest, SerializeBufferTooShort) {
    const std::array<std::uint16_t, 1> registers{0x000A};
    const auto response = modbus::response::ReadInputRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 3> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadInputRegistersTest, DeserializeSuccess) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x04}, std::byte{0x02}, std::byte{0x00}, std::byte{0x0A}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadInputRegisters::Deserialize(buffer, 1);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 2);

    const auto registers = response->GetValues();
    ASSERT_EQ(registers.size(), 1);
    EXPECT_EQ(registers[0], 0x000A);
    EXPECT_TRUE(buffer.empty());
}

UTEST(ResponseReadInputRegistersTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto response = modbus::response::ReadInputRegisters::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadInputRegistersTest, DeserializeBufferTooShortByteCount) {
    const std::array<std::byte, 1> raw_buffer{std::byte{0x04}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadInputRegisters::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadInputRegistersTest, DeserializeBufferTooShortData) {
    const std::array<std::byte, 3> raw_buffer{std::byte{0x04}, std::byte{0x02}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadInputRegisters::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadInputRegistersTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x03}, std::byte{0x02}, std::byte{0x00}, std::byte{0x0A}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadInputRegisters::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseReadInputRegistersTest, DeserializeInvalidByteCountOdd) {
    const std::array<std::byte, 3> raw_buffer{std::byte{0x04}, std::byte{0x01}, std::byte{0x0A}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadInputRegisters::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseReadInputRegistersTest, DeserializeInvalidQuantityZero) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x04}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadInputRegisters::Deserialize(buffer, 0);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadInputRegistersTest, DeserializeInvalidQuantityOverflow) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x04}, std::byte{0xFC}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadInputRegisters::Deserialize(buffer, 126);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadInputRegistersTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x04}, std::byte{0x02}, std::byte{0x00}, std::byte{0x0A}, std::byte{0xFF}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadInputRegisters::Deserialize(buffer, 1);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0xFF});
}
