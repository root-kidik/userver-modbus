#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/response/read_input_registers.hpp>

UTEST(ResponseReadInputRegistersTest, CreateSuccess) {
    const std::vector<std::uint16_t> registers{0x000A};
    const auto response = modbus::response::ReadInputRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 2);
    EXPECT_EQ(response->GetValues().size(), 1);
}

UTEST(ResponseReadInputRegistersTest, CreateMinQuantity) {
    const std::vector<std::uint16_t> registers{0x0001};
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
    const std::vector<std::uint16_t> registers{};
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

UTEST(ResponseReadInputRegistersTest, Serialize) {
    const std::vector<std::uint16_t> registers{0x000A};
    const auto response = modbus::response::ReadInputRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = response->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x04, 0x02, 0x00, 0x0A};
    EXPECT_EQ(buffer, expected);
}

UTEST(ResponseReadInputRegistersTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x04, 0x02, 0x00, 0x0A};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadInputRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 2);

    const auto registers = response->GetValues();
    ASSERT_EQ(registers.size(), 1);
    EXPECT_EQ(registers[0], 0x000A);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(ResponseReadInputRegistersTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadInputRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadInputRegistersTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x03, 0x02, 0x00, 0x0A};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadInputRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseReadInputRegistersTest, DeserializeBufferTooShortByteCount) {
    const std::vector<std::uint8_t> buffer{0x04};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadInputRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadInputRegistersTest, DeserializeInvalidByteCountOdd) {
    const std::vector<std::uint8_t> buffer{0x04, 0x01, 0x0A};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadInputRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseReadInputRegistersTest, DeserializeInvalidQuantityZero) {
    const std::vector<std::uint8_t> buffer{0x04, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadInputRegisters::Deserialize(it, buffer.cend(), 0);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadInputRegistersTest, DeserializeInvalidQuantityOverflow) {
    const std::vector<std::uint8_t> buffer{0x04, 0xFC};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadInputRegisters::Deserialize(it, buffer.cend(), 126);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadInputRegistersTest, DeserializeBufferTooShortData) {
    const std::vector<std::uint8_t> buffer{0x04, 0x02, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadInputRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadInputRegistersTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x04, 0x02, 0x00, 0x0A, 0xFF};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadInputRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kExtraDataAtEnd);
}
