#include <array>
#include <cstddef>
#include <span>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
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

UTEST(ResponseWriteMultipleCoilsTest, SerializeSuccess) {
    const auto response = modbus::response::WriteMultipleCoils::Create(0x0013, 0x000A);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x0F}, std::byte{0x00}, std::byte{0x13}, std::byte{0x00}, std::byte{0x0A}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(ResponseWriteMultipleCoilsTest, SerializeBufferTooShort) {
    const auto response = modbus::response::WriteMultipleCoils::Create(0x0013, 0x000A);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeSuccess) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x0F}, std::byte{0x00}, std::byte{0x13}, std::byte{0x00}, std::byte{0x0A}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x0013);
    EXPECT_EQ(response->GetQuantity(), 10);
    EXPECT_TRUE(buffer.empty());
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x0F}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeBufferTooShortQuantity) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x0F}, std::byte{0x00}, std::byte{0x13}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x10}, std::byte{0x00}, std::byte{0x13}, std::byte{0x00}, std::byte{0x0A}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeInvalidQuantity) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x0F}, std::byte{0x00}, std::byte{0x13}, std::byte{0x00}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializeAddressOverflow) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x0F}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0x00}, std::byte{0x02}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(ResponseWriteMultipleCoilsTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x0F},
        std::byte{0x00},
        std::byte{0x13},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0xFF}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0xFF});
}
