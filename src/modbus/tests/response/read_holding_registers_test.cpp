#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/response/read_holding_registers.hpp>

UTEST(ResponseReadHoldingRegistersTest, CreateSuccess) {
    const std::vector<std::uint16_t> registers{0x022B, 0x0000};
    const auto response = modbus::response::ReadHoldingRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 4);
    EXPECT_EQ(response->GetValues().size(), 2);
}

UTEST(ResponseReadHoldingRegistersTest, CreateMinQuantity) {
    const std::vector<std::uint16_t> registers{0x0001};
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
    const std::vector<std::uint16_t> registers{};
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

UTEST(ResponseReadHoldingRegistersTest, Serialize) {
    const std::vector<std::uint16_t> registers{0x022B, 0x0000};
    const auto response = modbus::response::ReadHoldingRegisters::Create(registers);
    ASSERT_TRUE(response.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = response->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x03, 0x04, 0x02, 0x2B, 0x00, 0x00};
    EXPECT_EQ(buffer, expected);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x03, 0x04, 0x02, 0x2B, 0x00, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(it, buffer.cend(), 2);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 4);

    const auto registers = response->GetValues();
    ASSERT_EQ(registers.size(), 2);
    EXPECT_EQ(registers[0], 0x022B);
    EXPECT_EQ(registers[1], 0x0000);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x04, 0x02, 0x12, 0x34};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeBufferTooShortByteCount) {
    const std::vector<std::uint8_t> buffer{0x03};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeInvalidByteCountOdd) {
    const std::vector<std::uint8_t> buffer{0x03, 0x03, 0x12, 0x34, 0x56};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeInvalidQuantityZero) {
    const std::vector<std::uint8_t> buffer{0x03, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(it, buffer.cend(), 0);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeInvalidQuantityOverflow) {
    const std::vector<std::uint8_t> buffer{0x03, 0xFC};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(it, buffer.cend(), 126);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeBufferTooShortData) {
    const std::vector<std::uint8_t> buffer{0x03, 0x04, 0x02, 0x2B, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(it, buffer.cend(), 2);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadHoldingRegistersTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x03, 0x02, 0x00, 0x0A, 0xEE};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadHoldingRegisters::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kExtraDataAtEnd);
}
