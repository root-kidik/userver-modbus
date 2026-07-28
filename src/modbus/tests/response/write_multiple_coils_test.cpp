#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/response/write_multiple_coils.hpp>

UTEST(ResponseWriteMultipleCoilsTest, CreateSuccess) {
    const auto response = modbus::response::WriteMultipleCoils::Create(0x0013, 10);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x0013);
    EXPECT_EQ(response->GetQuantity(), 10);
}

UTEST(ResponseWriteMultipleCoilsTest, CreateMinQuantity) {
    const auto response = modbus::response::WriteMultipleCoils::Create(0x0000, 1);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetQuantity(), 1);
}

UTEST(ResponseWriteMultipleCoilsTest, CreateMaxQuantity) {
    const auto response = modbus::response::WriteMultipleCoils::Create(0x0000, 1968);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetQuantity(), 1968);
}

UTEST(ResponseWriteMultipleCoilsTest, CreateInvalidQuantityZero) {
    const auto response = modbus::response::WriteMultipleCoils::Create(0x0000, 0);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseWriteMultipleCoilsTest, CreateInvalidQuantityOverflow) {
    const auto response = modbus::response::WriteMultipleCoils::Create(0x0000, 1969);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseWriteMultipleCoilsTest, CreateAddressOverflow) {
    const auto response = modbus::response::WriteMultipleCoils::Create(0xFFFF, 2);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(ResponseWriteMultipleCoilsTest, Serialize) {
    const auto response = modbus::response::WriteMultipleCoils::Create(0x0013, 0x000A);
    ASSERT_TRUE(response.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = response->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x0F, 0x00, 0x13, 0x00, 0x0A};
    EXPECT_EQ(buffer, expected);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x00, 0x0A};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x0013);
    EXPECT_EQ(response->GetQuantity(), 10);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeBufferTooShortQuantity) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x10, 0x00, 0x13, 0x00, 0x0A};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x00, 0x0A, 0xFF};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kExtraDataAtEnd);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeInvalidQuantity) {
    const std::vector<std::uint8_t> buffer{0x0F, 0x00, 0x13, 0x00, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeAddressOverflow) {
    const std::vector<std::uint8_t> buffer{0x0F, 0xFF, 0xFF, 0x00, 0x02};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kAddressOverflow);
}
