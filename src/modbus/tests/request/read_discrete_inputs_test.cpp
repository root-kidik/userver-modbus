#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/request/read_discrete_inputs.hpp>

UTEST(RequestReadDiscreteInputsTest, CreateSuccess) {
    const auto request = modbus::request::ReadDiscreteInputs::Create(0x0020, 15);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0020);
    EXPECT_EQ(request->GetQuantity(), 15);
}

UTEST(RequestReadDiscreteInputsTest, CreateMinQuantity) {
    const auto request = modbus::request::ReadDiscreteInputs::Create(0x0000, 1);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestReadDiscreteInputsTest, CreateMaxQuantity) {
    const auto request = modbus::request::ReadDiscreteInputs::Create(0x0000, 2000);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 2000);
}

UTEST(RequestReadDiscreteInputsTest, CreateInvalidQuantityUnderflow) {
    const auto request = modbus::request::ReadDiscreteInputs::Create(0x0020, 0);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadDiscreteInputsTest, CreateInvalidQuantityOverflow) {
    const auto request = modbus::request::ReadDiscreteInputs::Create(0x0020, 2001);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadDiscreteInputsTest, CreateAddressOverflow) {
    const auto request = modbus::request::ReadDiscreteInputs::Create(0xFFFF, 2);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestReadDiscreteInputsTest, Serialize) {
    const auto request = modbus::request::ReadDiscreteInputs::Create(0x0020, 0x000F);
    ASSERT_TRUE(request.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = request->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x02, 0x00, 0x20, 0x00, 0x0F};
    EXPECT_EQ(buffer, expected);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x02, 0x00, 0x20, 0x00, 0x0F};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(it, buffer.cend());
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0020);
    EXPECT_EQ(request->GetQuantity(), 15);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(RequestReadDiscreteInputsTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x02, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeBufferTooShortQuantity) {
    const std::vector<std::uint8_t> buffer{0x02, 0x00, 0x20, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x01, 0x00, 0x20, 0x00, 0x0F};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x02, 0x00, 0x20, 0x00, 0x0F, 0xAA};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kExtraDataAtEnd);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeInvalidQuantity) {
    const std::vector<std::uint8_t> buffer{0x02, 0x00, 0x20, 0x00, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeAddressOverflow) {
    const std::vector<std::uint8_t> buffer{0x02, 0xFF, 0xFF, 0x00, 0x02};
    auto it = buffer.cbegin();

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}
