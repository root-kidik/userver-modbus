#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/request/read_holding_registers.hpp>

UTEST(RequestReadHoldingRegistersTest, CreateSuccess) {
    const auto request = modbus::request::ReadHoldingRegisters::Create(0x006B, 3);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x006B);
    EXPECT_EQ(request->GetQuantity(), 3);
}

UTEST(RequestReadHoldingRegistersTest, CreateMinQuantity) {
    const auto request = modbus::request::ReadHoldingRegisters::Create(0x0000, 1);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestReadHoldingRegistersTest, CreateMaxQuantity) {
    const auto request = modbus::request::ReadHoldingRegisters::Create(0x0000, 125);
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

UTEST(RequestReadHoldingRegistersTest, Serialize) {
    const auto request = modbus::request::ReadHoldingRegisters::Create(0x006B, 0x0003);
    ASSERT_TRUE(request.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = request->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x03, 0x00, 0x6B, 0x00, 0x03};
    EXPECT_EQ(buffer, expected);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x03, 0x00, 0x6B, 0x00, 0x03};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x006B);
    EXPECT_EQ(request->GetQuantity(), 3);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(RequestReadHoldingRegistersTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x03, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeBufferTooShortQuantity) {
    const std::vector<std::uint8_t> buffer{0x03, 0x00, 0x6B, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x04, 0x00, 0x6B, 0x00, 0x03};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x03, 0x00, 0x6B, 0x00, 0x03, 0x01};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kExtraDataAtEnd);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeInvalidQuantity) {
    const std::vector<std::uint8_t> buffer{0x03, 0x00, 0x6B, 0x00, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadHoldingRegistersTest, DeserializeAddressOverflow) {
    const std::vector<std::uint8_t> buffer{0x03, 0xFF, 0xFF, 0x00, 0x02};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}
