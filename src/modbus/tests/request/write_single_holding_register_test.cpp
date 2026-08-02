#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/request/write_single_holding_register.hpp>

UTEST(RequestWriteSingleHoldingRegisterTest, CreateSuccess) {
    const auto request = modbus::request::WriteSingleHoldingRegister::Create(0x0001, 0x0003);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0001);
    EXPECT_EQ(request->GetValue(), 0x0003);
}

UTEST(RequestWriteSingleHoldingRegisterTest, Serialize) {
    const auto request = modbus::request::WriteSingleHoldingRegister::Create(0x0001, 0x0003);
    ASSERT_TRUE(request.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = request->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x06, 0x00, 0x01, 0x00, 0x03};
    EXPECT_EQ(buffer, expected);
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x06, 0x00, 0x01, 0x00, 0x03};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0001);
    EXPECT_EQ(request->GetValue(), 0x0003);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x06, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializeBufferTooShortValue) {
    const std::vector<std::uint8_t> buffer{0x06, 0x00, 0x01, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x01, 0x00, 0x03};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestWriteSingleHoldingRegisterTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x06, 0x00, 0x01, 0x00, 0x03, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleHoldingRegister::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kExtraDataAtEnd);
}
