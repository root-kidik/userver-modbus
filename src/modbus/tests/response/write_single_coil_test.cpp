#include <array>
#include <cstddef>
#include <span>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
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

UTEST(ResponseWriteSingleCoilTest, SerializeOnSuccess) {
    const auto response = modbus::response::WriteSingleCoil::Create(0x000A, modbus::Coil::kOn);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x05}, std::byte{0x00}, std::byte{0x0A}, std::byte{0xFF}, std::byte{0x00}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(ResponseWriteSingleCoilTest, SerializeOffSuccess) {
    const auto response = modbus::response::WriteSingleCoil::Create(0x000A, modbus::Coil::kOff);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x05}, std::byte{0x00}, std::byte{0x0A}, std::byte{0x00}, std::byte{0x00}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(ResponseWriteSingleCoilTest, SerializeBufferTooShort) {
    const auto response = modbus::response::WriteSingleCoil::Create(0x000A, modbus::Coil::kOn);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleCoilTest, DeserializeSuccessOn) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x05}, std::byte{0x00}, std::byte{0x0A}, std::byte{0xFF}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleCoil::Deserialize(buffer);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x000A);
    EXPECT_EQ(response->GetValue(), modbus::Coil::kOn);
    EXPECT_TRUE(buffer.empty());
}

UTEST(ResponseWriteSingleCoilTest, DeserializeSuccessOff) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x05}, std::byte{0x00}, std::byte{0x0A}, std::byte{0x00}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleCoil::Deserialize(buffer);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetAddress(), 0x000A);
    EXPECT_EQ(response->GetValue(), modbus::Coil::kOff);
    EXPECT_TRUE(buffer.empty());
}

UTEST(ResponseWriteSingleCoilTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto response = modbus::response::WriteSingleCoil::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleCoilTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x05}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleCoil::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleCoilTest, DeserializeBufferTooShortValue) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x05}, std::byte{0x00}, std::byte{0x0A}, std::byte{0xFF}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleCoil::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseWriteSingleCoilTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x06}, std::byte{0x00}, std::byte{0x0A}, std::byte{0xFF}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleCoil::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseWriteSingleCoilTest, DeserializeInvalidCoilValue) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x05}, std::byte{0x00}, std::byte{0x0A}, std::byte{0xAB}, std::byte{0xCD}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleCoil::Deserialize(buffer);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseWriteSingleCoilTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x05},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0xFF},
        std::byte{0x00},
        std::byte{0x12}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::WriteSingleCoil::Deserialize(buffer);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0x12});
}
