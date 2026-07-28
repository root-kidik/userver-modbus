#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/request/read_input_registers.hpp>

UTEST(RequestReadInputRegistersTest, CreateSuccess) {
    const auto request = modbus::request::ReadInputRegisters::Create(0x0008, 1);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0008);
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestReadInputRegistersTest, CreateMinQuantity) {
    const auto request = modbus::request::ReadInputRegisters::Create(0x0000, 1);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestReadInputRegistersTest, CreateMaxQuantity) {
    const auto request = modbus::request::ReadInputRegisters::Create(0x0000, 125);
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

UTEST(RequestReadInputRegistersTest, Serialize) {
    const auto request = modbus::request::ReadInputRegisters::Create(0x0008, 0x0001);
    ASSERT_TRUE(request.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = request->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x04, 0x00, 0x08, 0x00, 0x01};
    EXPECT_EQ(buffer, expected);
}

UTEST(RequestReadInputRegistersTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x04, 0x00, 0x08, 0x00, 0x01};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadInputRegisters::Deserialize(it, buffer.cend());
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0008);
    EXPECT_EQ(request->GetQuantity(), 1);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(RequestReadInputRegistersTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadInputRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadInputRegistersTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x04, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadInputRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadInputRegistersTest, DeserializeBufferTooShortQuantity) {
    const std::vector<std::uint8_t> buffer{0x04, 0x00, 0x08, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadInputRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadInputRegistersTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x03, 0x00, 0x08, 0x00, 0x01};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadInputRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestReadInputRegistersTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x04, 0x00, 0x08, 0x00, 0x01, 0x99};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadInputRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kExtraDataAtEnd);
}

UTEST(RequestReadInputRegistersTest, DeserializeInvalidQuantity) {
    const std::vector<std::uint8_t> buffer{0x04, 0x00, 0x08, 0x00, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadInputRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadInputRegistersTest, DeserializeAddressOverflow) {
    const std::vector<std::uint8_t> buffer{0x04, 0xFF, 0xFF, 0x00, 0x02};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadInputRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}
