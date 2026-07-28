#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/request/write_multiple_coils.hpp>

UTEST(RequestWriteMultipleCoilsTest, CreateSuccess) {
    const std::vector<modbus::Coil> coils{
        modbus::Coil::kOn, modbus::Coil::kOff, modbus::Coil::kOn, modbus::Coil::kOff,
        modbus::Coil::kOff, modbus::Coil::kOn, modbus::Coil::kOn, modbus::Coil::kOff,
        modbus::Coil::kOn, modbus::Coil::kOff
    };
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0013, coils);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0013);
    EXPECT_EQ(request->GetQuantity(), 10);
    EXPECT_EQ(request->GetByteCount(), 2);
    EXPECT_EQ(request->GetValues().size(), 10);
    EXPECT_EQ(request->GetCoils().size(), 10);
}

UTEST(RequestWriteMultipleCoilsTest, CreateMinQuantity) {
    const std::vector<modbus::Coil> coils{modbus::Coil::kOn};
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0000, coils);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestWriteMultipleCoilsTest, CreateMaxQuantity) {
    const std::vector<modbus::Coil> coils(1968, modbus::Coil::kOn);
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0000, coils);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1968);
}

UTEST(RequestWriteMultipleCoilsTest, CreateInvalidQuantityZero) {
    const std::vector<modbus::Coil> coils{};
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0000, coils);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleCoilsTest, CreateInvalidQuantityOverflow) {
    const std::vector<modbus::Coil> coils(1969, modbus::Coil::kOn);
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0000, coils);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleCoilsTest, CreateAddressOverflow) {
    const std::vector<modbus::Coil> coils(2, modbus::Coil::kOn);
    const auto request = modbus::request::WriteMultipleCoils::Create(0xFFFF, coils);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestWriteMultipleCoilsTest, CreateInvalidValue) {
    const std::vector<modbus::Coil> coils{static_cast<modbus::Coil>(0x1234)};
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0000, coils);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidValue);
}

UTEST(RequestWriteMultipleCoilsTest, Serialize) {
    const std::vector<modbus::Coil> coils{
        modbus::Coil::kOn, modbus::Coil::kOff, modbus::Coil::kOn, modbus::Coil::kOff,
        modbus::Coil::kOff, modbus::Coil::kOn, modbus::Coil::kOn, modbus::Coil::kOff,
        modbus::Coil::kOn, modbus::Coil::kOff
    };
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0013, coils);
    ASSERT_TRUE(request.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = request->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x0F, 0x00, 0x13, 0x00, 0x0A, 0x02, 0x65, 0x01};
    EXPECT_EQ(buffer, expected);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x00, 0x0A, 0x02, 0x65, 0x01};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0013);
    EXPECT_EQ(request->GetQuantity(), 10);
    EXPECT_EQ(request->GetByteCount(), 2);

    const auto coils = request->GetCoils();
    ASSERT_EQ(coils.size(), 10);
    EXPECT_EQ(coils[0], modbus::Coil::kOn);
    EXPECT_EQ(coils[1], modbus::Coil::kOff);
    EXPECT_EQ(coils[2], modbus::Coil::kOn);
    EXPECT_EQ(coils[3], modbus::Coil::kOff);
    EXPECT_EQ(coils[4], modbus::Coil::kOff);
    EXPECT_EQ(coils[5], modbus::Coil::kOn);
    EXPECT_EQ(coils[6], modbus::Coil::kOn);
    EXPECT_EQ(coils[7], modbus::Coil::kOff);
    EXPECT_EQ(coils[8], modbus::Coil::kOn);
    EXPECT_EQ(coils[9], modbus::Coil::kOff);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeBufferTooShortQuantity) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x13, 0x00, 0x0A, 0x02, 0x65, 0x01};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeInvalidQuantityUnderflow) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x00, 0x00, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeInvalidQuantityOverflow) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x07, 0xB1, 0xF6};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeAddressOverflow) {
    const std::vector<std::uint8_t> buffer{0x0F, 0xFF, 0xFF, 0x00, 0x02, 0x01, 0x03};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeBufferTooShortByteCount) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x00, 0x0A};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeInvalidByteCount) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x00, 0x0A, 0x03, 0x65, 0x01, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeBufferTooShortDataBytes) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x00, 0x0A, 0x02, 0x65};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x00, 0x0A, 0x02, 0x65, 0x01, 0xFF};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kExtraDataAtEnd);
}
