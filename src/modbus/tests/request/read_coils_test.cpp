#include <array>
#include <cstddef>
#include <span>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
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

UTEST(RequestReadCoilsTest, SerializeSuccess) {
    const auto request = modbus::request::ReadCoils::Create(0x0123, 0x0045);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x01}, std::byte{0x01}, std::byte{0x23}, std::byte{0x00}, std::byte{0x45}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(RequestReadCoilsTest, SerializeBufferTooShort) {
    const auto request = modbus::request::ReadCoils::Create(0x0123, 0x0045);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadCoilsTest, DeserializeSuccess) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x01}, std::byte{0x00}, std::byte{0x10}, std::byte{0x00}, std::byte{0x0A}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadCoils::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0010);
    EXPECT_EQ(request->GetQuantity(), 10);
    EXPECT_TRUE(buffer.empty());
}

UTEST(RequestReadCoilsTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto request = modbus::request::ReadCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadCoilsTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x01}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadCoilsTest, DeserializeBufferTooShortQuantity) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x01}, std::byte{0x00}, std::byte{0x10}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadCoilsTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x02}, std::byte{0x00}, std::byte{0x10}, std::byte{0x00}, std::byte{0x0A}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestReadCoilsTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x10},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0xFF}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadCoils::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0xFF});
}

UTEST(RequestReadCoilsTest, DeserializeInvalidQuantity) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x01}, std::byte{0x00}, std::byte{0x10}, std::byte{0x00}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadCoilsTest, DeserializeAddressOverflow) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x01}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0x00}, std::byte{0x02}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}
