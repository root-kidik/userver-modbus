#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/request/write_single_coil.hpp>

UTEST(RequestWriteSingleCoilTest, CreateSuccessOn) {
    const auto request = modbus::request::WriteSingleCoil::Create(0x0005, modbus::Coil::kOn);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0005);
    EXPECT_EQ(request->GetValue(), modbus::Coil::kOn);
}

UTEST(RequestWriteSingleCoilTest, CreateSuccessOff) {
    const auto request = modbus::request::WriteSingleCoil::Create(0x0005, modbus::Coil::kOff);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0005);
    EXPECT_EQ(request->GetValue(), modbus::Coil::kOff);
}

UTEST(RequestWriteSingleCoilTest, CreateInvalidValue) {
    const auto request = modbus::request::WriteSingleCoil::Create(0x0005, static_cast<modbus::Coil>(0x1234));
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidValue);
}

UTEST(RequestWriteSingleCoilTest, SerializeOn) {
    const auto request = modbus::request::WriteSingleCoil::Create(0x0005, modbus::Coil::kOn);
    ASSERT_TRUE(request.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = request->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x05, 0x00, 0x05, 0xFF, 0x00};
    EXPECT_EQ(buffer, expected);
}

UTEST(RequestWriteSingleCoilTest, SerializeOff) {
    const auto request = modbus::request::WriteSingleCoil::Create(0x0005, modbus::Coil::kOff);
    ASSERT_TRUE(request.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = request->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x05, 0x00, 0x05, 0x00, 0x00};
    EXPECT_EQ(buffer, expected);
}

UTEST(RequestWriteSingleCoilTest, DeserializeSuccessOn) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x05, 0xFF, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0005);
    EXPECT_EQ(request->GetValue(), modbus::Coil::kOn);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(RequestWriteSingleCoilTest, DeserializeSuccessOff) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x05, 0x00, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0005);
    EXPECT_EQ(request->GetValue(), modbus::Coil::kOff);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(RequestWriteSingleCoilTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleCoilTest, DeserializeBufferTooShortAddress) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleCoilTest, DeserializeBufferTooShortValue) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x05, 0xFF};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleCoilTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x06, 0x00, 0x05, 0xFF, 0x00};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestWriteSingleCoilTest, DeserializeInvalidCoilValue) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x05, 0x12, 0x34};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidValue);
}

UTEST(RequestWriteSingleCoilTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x05, 0x00, 0x05, 0xFF, 0x00, 0x01};
    auto it = buffer.cbegin();

    const auto request = modbus::request::WriteSingleCoil::Deserialize(it, buffer.cend());
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kExtraDataAtEnd);
}
