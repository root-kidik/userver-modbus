#include <vector>

#include <userver/utest/utest.hpp>

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

UTEST(ResponseWriteMultipleHoldingRegistersTest, Serialize) {
    const auto response = modbus::response::WriteMultipleHoldingRegisters::Create(0x0001, 0x0002);
    ASSERT_TRUE(response.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = response->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x10, 0x00, 0x01, 0x00, 0x02};
    EXPECT_EQ(buffer, expected);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00, 0x02};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x0001);
    EXPECT_EQ(response->GetQuantity(), 2);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortQuantity) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x01, 0x00, 0x02};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00, 0x02, 0xCD};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kExtraDataAtEnd);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeInvalidQuantity) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseWriteMultipleHoldingRegistersTest, DeserializeAddressOverflow) {
    const std::vector<std::uint8_t> buffer{0x10, 0xFF, 0xFF, 0x00, 0x02};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kAddressOverflow);
}
