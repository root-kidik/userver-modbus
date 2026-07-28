#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/response/read_coils.hpp>

UTEST(ResponseReadCoilsTest, CreateSuccess) {
    const std::vector<modbus::Coil> coils{modbus::Coil::kOn, modbus::Coil::kOff, modbus::Coil::kOn};
    const auto response = modbus::response::ReadCoils::Create(coils);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 1);
    EXPECT_EQ(response->GetValues().size(), 3);
}

UTEST(ResponseReadCoilsTest, CreateMinQuantity) {
    const std::vector<modbus::Coil> coils{modbus::Coil::kOn};
    const auto response = modbus::response::ReadCoils::Create(coils);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetValues().size(), 1);
}

UTEST(ResponseReadCoilsTest, CreateMaxQuantity) {
    const std::vector<modbus::Coil> coils(2000, modbus::Coil::kOn);
    const auto response = modbus::response::ReadCoils::Create(coils);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetValues().size(), 2000);
}

UTEST(ResponseReadCoilsTest, CreateInvalidQuantityZero) {
    const std::vector<modbus::Coil> coils{};
    const auto response = modbus::response::ReadCoils::Create(coils);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadCoilsTest, CreateInvalidQuantityOverflow) {
    const std::vector<modbus::Coil> coils(2001, modbus::Coil::kOn);
    const auto response = modbus::response::ReadCoils::Create(coils);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadCoilsTest, CreateInvalidValue) {
    const std::vector<modbus::Coil> coils{static_cast<modbus::Coil>(0x1234)};
    const auto response = modbus::response::ReadCoils::Create(coils);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseReadCoilsTest, Serialize) {
    const std::vector<modbus::Coil> coils{
        modbus::Coil::kOn, modbus::Coil::kOff, modbus::Coil::kOn, modbus::Coil::kOff,
        modbus::Coil::kOff, modbus::Coil::kOn, modbus::Coil::kOn, modbus::Coil::kOff,
        modbus::Coil::kOn
    };
    const auto response = modbus::response::ReadCoils::Create(coils);
    ASSERT_TRUE(response.has_value());

    std::vector<std::uint8_t> buffer;
    std::ignore = response->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x01, 0x02, 0x65, 0x01};
    EXPECT_EQ(buffer, expected);
}

UTEST(ResponseReadCoilsTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x01, 0x02, 0x65, 0x01};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadCoils::Deserialize(it, buffer.cend(), 9);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 2);

    const auto values = response->GetValues();
    ASSERT_EQ(values.size(), 9);
    EXPECT_EQ(values[0], modbus::Coil::kOn);
    EXPECT_EQ(values[1], modbus::Coil::kOff);
    EXPECT_EQ(values[2], modbus::Coil::kOn);
    EXPECT_EQ(values[3], modbus::Coil::kOff);
    EXPECT_EQ(values[4], modbus::Coil::kOff);
    EXPECT_EQ(values[5], modbus::Coil::kOn);
    EXPECT_EQ(values[6], modbus::Coil::kOn);
    EXPECT_EQ(values[7], modbus::Coil::kOff);
    EXPECT_EQ(values[8], modbus::Coil::kOn);
    EXPECT_EQ(it, buffer.cend());
}

UTEST(ResponseReadCoilsTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadCoils::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadCoilsTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x02, 0x01, 0x01};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadCoils::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseReadCoilsTest, DeserializeBufferTooShortByteCount) {
    const std::vector<std::uint8_t> buffer{0x01};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadCoils::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadCoilsTest, DeserializeInvalidQuantityZero) {
    const std::vector<std::uint8_t> buffer{0x01, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadCoils::Deserialize(it, buffer.cend(), 0);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadCoilsTest, DeserializeInvalidQuantityOverflow) {
    const std::vector<std::uint8_t> buffer{0x01, 0xFA};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadCoils::Deserialize(it, buffer.cend(), 2001);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadCoilsTest, DeserializeInvalidByteCount) {
    const std::vector<std::uint8_t> buffer{0x01, 0x01, 0x65};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadCoils::Deserialize(it, buffer.cend(), 9);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseReadCoilsTest, DeserializeBufferTooShortData) {
    const std::vector<std::uint8_t> buffer{0x01, 0x02, 0x65};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadCoils::Deserialize(it, buffer.cend(), 9);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadCoilsTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x01, 0x01, 0x01, 0xFF};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadCoils::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kExtraDataAtEnd);
}
