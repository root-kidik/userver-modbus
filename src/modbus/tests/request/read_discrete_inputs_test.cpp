#include <array>
#include <cstddef>
#include <span>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/request/read_discrete_inputs.hpp>

UTEST(RequestReadDiscreteInputsTest, CreateSuccess) {
    const auto request = modbus::request::ReadDiscreteInputs::Create(0x0020, 15);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0020);
    EXPECT_EQ(request->GetQuantity(), 15);
}

UTEST(RequestReadDiscreteInputsTest, CreateMinQuantity) {
    const auto request =
        modbus::request::ReadDiscreteInputs::Create(0x0000, modbus::request::ReadDiscreteInputs::kMinQuantity);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestReadDiscreteInputsTest, CreateMaxQuantity) {
    const auto request =
        modbus::request::ReadDiscreteInputs::Create(0x0000, modbus::request::ReadDiscreteInputs::kMaxQuantity);
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

UTEST(RequestReadDiscreteInputsTest, SerializeSuccess) {
    const auto request = modbus::request::ReadDiscreteInputs::Create(0x0020, 0x000F);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x02}, std::byte{0x00}, std::byte{0x20}, std::byte{0x00}, std::byte{0x0F}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(RequestReadDiscreteInputsTest, SerializeBufferTooShort) {
    const auto request = modbus::request::ReadDiscreteInputs::Create(0x0020, 0x000F);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeSuccess) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x02}, std::byte{0x00}, std::byte{0x20}, std::byte{0x00}, std::byte{0x0F}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0020);
    EXPECT_EQ(request->GetQuantity(), 15);
    EXPECT_TRUE(buffer.empty());
}

UTEST(RequestReadDiscreteInputsTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x02}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeBufferTooShortQuantity) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x02}, std::byte{0x00}, std::byte{0x20}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x01}, std::byte{0x00}, std::byte{0x20}, std::byte{0x00}, std::byte{0x0F}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestReadDiscreteInputsTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x02},
        std::byte{0x00},
        std::byte{0x20},
        std::byte{0x00},
        std::byte{0x0F},
        std::byte{0xAA}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0xAA});
}

UTEST(RequestReadDiscreteInputsTest, DeserializeInvalidQuantity) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x02}, std::byte{0x00}, std::byte{0x20}, std::byte{0x00}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestReadDiscreteInputsTest, DeserializeAddressOverflow) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x02}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0x00}, std::byte{0x02}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::ReadDiscreteInputs::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}
