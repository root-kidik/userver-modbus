#include <array>
#include <cstddef>
#include <span>
#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/response/read_coils.hpp>

UTEST(ResponseReadCoilsTest, CreateSuccess) {
    const std::array coils{modbus::Coil::kOn, modbus::Coil::kOff, modbus::Coil::kOn};
    const auto response = modbus::response::ReadCoils::Create(coils);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 1);
    EXPECT_EQ(response->GetValues().size(), 3);
}

UTEST(ResponseReadCoilsTest, CreateMinQuantity) {
    const std::array coils{modbus::Coil::kOn};
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
    const std::array<modbus::Coil, 0> coils{};
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

UTEST(ResponseReadCoilsTest, SerializeSuccess) {
    const std::array coils{
        modbus::Coil::kOn,
        modbus::Coil::kOff,
        modbus::Coil::kOn,
        modbus::Coil::kOff,
        modbus::Coil::kOff,
        modbus::Coil::kOn,
        modbus::Coil::kOn,
        modbus::Coil::kOff,
        modbus::Coil::kOn
    };
    const auto response = modbus::response::ReadCoils::Create(coils);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 4> expected{std::byte{0x01}, std::byte{0x02}, std::byte{0x65}, std::byte{0x01}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(ResponseReadCoilsTest, SerializeBufferTooShort) {
    const std::array coils{modbus::Coil::kOn, modbus::Coil::kOff, modbus::Coil::kOn};
    const auto response = modbus::response::ReadCoils::Create(coils);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 2> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadCoilsTest, DeserializeSuccess) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x01}, std::byte{0x02}, std::byte{0x65}, std::byte{0x01}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadCoils::Deserialize(buffer, 9);
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
    EXPECT_TRUE(buffer.empty());
}

UTEST(ResponseReadCoilsTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto response = modbus::response::ReadCoils::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadCoilsTest, DeserializeBufferTooShortByteCount) {
    const std::array<std::byte, 1> raw_buffer{std::byte{0x01}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadCoils::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadCoilsTest, DeserializeBufferTooShortData) {
    const std::array<std::byte, 3> raw_buffer{std::byte{0x01}, std::byte{0x02}, std::byte{0x65}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadCoils::Deserialize(buffer, 9);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadCoilsTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 3> raw_buffer{std::byte{0x02}, std::byte{0x01}, std::byte{0x01}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadCoils::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseReadCoilsTest, DeserializeInvalidQuantityZero) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x01}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadCoils::Deserialize(buffer, 0);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadCoilsTest, DeserializeInvalidQuantityOverflow) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x01}, std::byte{0xFA}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadCoils::Deserialize(buffer, 2001);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadCoilsTest, DeserializeInvalidByteCount) {
    const std::array<std::byte, 3> raw_buffer{std::byte{0x01}, std::byte{0x01}, std::byte{0x65}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadCoils::Deserialize(buffer, 9);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseReadCoilsTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x01}, std::byte{0x01}, std::byte{0x01}, std::byte{0xFF}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadCoils::Deserialize(buffer, 1);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0xFF});
}
