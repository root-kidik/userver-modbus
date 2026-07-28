#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/response/write_single_coil.hpp>

UTEST(ResponseWriteSingleCoilTest, CreateSuccessOn) {
    const auto response = modbus::response::WriteSingleCoil::Create(0x000A, modbus::Coil::kOn);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x000A);
    EXPECT_EQ(response->GetValue(), modbus::Coil::kOn);
}

UTEST(ResponseWriteSingleCoilTest, CreateSuccessOff) {
    const auto response = modbus::response::WriteSingleCoil::Create(0x000A, modbus::Coil::kOff);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x000A);
    EXPECT_EQ(response->GetValue(), modbus::Coil::kOff);
}

UTEST(ResponseWriteSingleCoilTest, CreateInvalidValue) {
    const auto response = modbus::response::WriteSingleCoil::Create(0x000A, static_cast<modbus::Coil>(0x1234));
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseWriteSingleCoilTest, SerializeOn) {
    const auto response = modbus::response::WriteSingleCoil::Create(0x000A, modbus::Coil::kOn);
    ASSERT_TRUE(response.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = response->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x05, 0x00, 0x0A, 0xFF, 0x00};
    EXPECT_EQ(buffer, expected);
}

UTEST(ResponseWriteSingleCoilTest, SerializeOff) {
    const auto response = modbus::response::WriteSingleCoil::Create(0x000A, modbus::Coil::kOff);
    ASSERT_TRUE(response.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = response->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x05, 0x00, 0x0A, 0x00, 0x00};
    EXPECT_EQ(buffer, expected);
}

UTEST(ResponseWriteSingleCoilTest, DeserializeSuccessOn) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x0A, 0xFF, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x000A);
    EXPECT_EQ(response->GetValue(), modbus::Coil::kOn);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(ResponseWriteSingleCoilTest, DeserializeSuccessOff) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x0A, 0x00, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x000A);
    EXPECT_EQ(response->GetValue(), modbus::Coil::kOff);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(ResponseWriteSingleCoilTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleCoilTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleCoilTest, DeserializeBufferTooShortValue) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x0A, 0xFF};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleCoilTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x06, 0x00, 0x0A, 0xFF, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseWriteSingleCoilTest, DeserializeInvalidCoilValue) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x0A, 0xAB, 0xCD};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseWriteSingleCoilTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x0A, 0xFF, 0x00, 0x12};
    auto it = buffer.cbegin();

    const auto response = modbus::response::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kExtraDataAtEnd);
}
