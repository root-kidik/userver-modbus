#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/request/write_multiple_holding_registers.hpp>

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateSuccess) {
    const std::vector<std::uint16_t> registers{0x000A, 0x0102};
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0001, registers);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0001);
    EXPECT_EQ(request->GetQuantity(), 2);
    EXPECT_EQ(request->GetByteCount(), 4);
    EXPECT_EQ(request->GetValues().size(), 2);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateMinQuantity) {
    const std::vector<std::uint16_t> registers{0x1234};
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0000, registers);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateMaxQuantity) {
    const std::vector<std::uint16_t> registers(123, 0x1234);
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0000, registers);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 123);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateInvalidQuantityZero) {
    const std::vector<std::uint16_t> registers{};
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0001, registers);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateInvalidQuantityOverflow) {
    const std::vector<std::uint16_t> registers(124, 0x1234);
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0001, registers);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateAddressOverflow) {
    const std::vector<std::uint16_t> registers(2, 0x1234);
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0xFFFF, registers);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, Serialize) {
    const std::vector<std::uint16_t> registers{0x000A, 0x0102};
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0001, registers);
    ASSERT_TRUE(request.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = request->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x10, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x0A, 0x01, 0x02};
    EXPECT_EQ(buffer, expected);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x0A, 0x01, 0x02};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0001);
    EXPECT_EQ(request->GetQuantity(), 2);
    EXPECT_EQ(request->GetByteCount(), 4);

    const auto registers = request->GetValues();
    ASSERT_EQ(registers.size(), 2);
    EXPECT_EQ(registers[0], 0x000A);
    EXPECT_EQ(registers[1], 0x0102);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortQuantity) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x0A, 0x01, 0x02};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeInvalidQuantityUnderflow) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00, 0x00, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeInvalidQuantityOverflow) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00, 0x7C, 0xF8};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeAddressOverflow) {
    const std::vector<std::uint8_t> buffer{0x10, 0xFF, 0xFF, 0x00, 0x02, 0x04, 0x00, 0x01, 0x00, 0x02};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortByteCount) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00, 0x02};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeInvalidByteCount) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00, 0x02, 0x05, 0x00, 0x0A, 0x01, 0x02};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortDataBytes) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x0A, 0x01};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x0A, 0x01, 0x02, 0xBB};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kExtraDataAtEnd);
}
