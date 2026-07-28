#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/request/read_coils.hpp>

UTEST(RequestReadCoilsTest, CreateSuccess) {
    const auto request = modbus::request::ReadCoils::Create(0x0010, 10);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0010);
    EXPECT_EQ(request->GetQuantity(), 10);
}

UTEST(RequestReadCoilsTest, CreateMinQuantity) {
    const auto request = modbus::request::ReadCoils::Create(0x0000, modbus::request::ReadCoils::kMinQuantity);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestReadCoilsTest, CreateMaxQuantity) {
    const auto request = modbus::request::ReadCoils::Create(0x0000, modbus::request::ReadCoils::kMaxQuantity);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 2000);
}

UTEST(RequestReadCoilsTest, CreateInvalidQuantityUnderflow) {
    const auto request = modbus::request::ReadCoils::Create(0x0010, 0);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadCoilsTest, CreateInvalidQuantityOverflow) {
    const auto request = modbus::request::ReadCoils::Create(0x0010, 2001);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadCoilsTest, CreateAddressOverflow) {
    const auto request = modbus::request::ReadCoils::Create(0xFFFF, 2);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestReadCoilsTest, Serialize) {
    const auto request = modbus::request::ReadCoils::Create(0x0123, 0x0045);
    ASSERT_TRUE(request.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = request->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x01, 0x01, 0x23, 0x00, 0x45};
    EXPECT_EQ(buffer, expected);
}

UTEST(RequestReadCoilsTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x01, 0x00, 0x10, 0x00, 0x0A};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadCoils::Deserialize(it, buffer.cend());
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0010);
    EXPECT_EQ(request->GetQuantity(), 10);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(RequestReadCoilsTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadCoilsTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x01, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadCoilsTest, DeserializeBufferTooShortQuantity) {
    const std::vector<std::uint8_t> buffer{0x01, 0x00, 0x10, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadCoilsTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x02, 0x00, 0x10, 0x00, 0x0A};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestReadCoilsTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x01, 0x00, 0x10, 0x00, 0x0A, 0xFF};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kExtraDataAtEnd);
}

UTEST(RequestReadCoilsTest, DeserializeInvalidQuantity) {
    const std::vector<std::uint8_t> buffer{0x01, 0x00, 0x10, 0x00, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadCoilsTest, DeserializeAddressOverflow) {
    const std::vector<std::uint8_t> buffer{0x01, 0xFF, 0xFF, 0x00, 0x02};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}
