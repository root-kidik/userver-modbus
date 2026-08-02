#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/response/write_single_holding_register.hpp>

UTEST(ResponseWriteSingleHoldingRegisterTest, CreateSuccess) {
    const auto response = modbus::response::WriteSingleHoldingRegister::Create(0x0001, 0x0003);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x0001);
    EXPECT_EQ(response->GetValue(), 0x0003);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, Serialize) {
    const auto response = modbus::response::WriteSingleHoldingRegister::Create(0x0001, 0x0003);
    ASSERT_TRUE(response.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = response->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x06, 0x00, 0x01, 0x00, 0x03};
    EXPECT_EQ(buffer, expected);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x06, 0x00, 0x01, 0x00, 0x03};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x0001);
    EXPECT_EQ(response->GetValue(), 0x0003);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x06, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializeBufferTooShortValue) {
    const std::vector<std::uint8_t> buffer{0x06, 0x00, 0x01, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x01, 0x00, 0x03};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseWriteSingleHoldingRegisterTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x06, 0x00, 0x01, 0x00, 0x03, 0x44};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kExtraDataAtEnd);
}
