#include <array>
#include <cstddef>
#include <span>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
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

    std::array<std::byte, 5> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x05}, std::byte{0x00}, std::byte{0x05}, std::byte{0xFF}, std::byte{0x00}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(RequestWriteSingleCoilTest, SerializeOff) {
    const auto request = modbus::request::WriteSingleCoil::Create(0x0005, modbus::Coil::kOff);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 5> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 5>
        expected{std::byte{0x05}, std::byte{0x00}, std::byte{0x05}, std::byte{0x00}, std::byte{0x00}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(RequestWriteSingleCoilTest, SerializeBufferTooShort) {
    const auto request = modbus::request::WriteSingleCoil::Create(0x0005, modbus::Coil::kOn);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleCoilTest, DeserializeSuccessOn) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x05}, std::byte{0x00}, std::byte{0x05}, std::byte{0xFF}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleCoil::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0005);
    EXPECT_EQ(request->GetValue(), modbus::Coil::kOn);
    EXPECT_TRUE(buffer.empty());
}

UTEST(RequestWriteSingleCoilTest, DeserializeSuccessOff) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x05}, std::byte{0x00}, std::byte{0x05}, std::byte{0x00}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleCoil::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0005);
    EXPECT_EQ(request->GetValue(), modbus::Coil::kOff);
    EXPECT_TRUE(buffer.empty());
}

UTEST(RequestWriteSingleCoilTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto request = modbus::request::WriteSingleCoil::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleCoilTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x05}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleCoil::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleCoilTest, DeserializeBufferTooShortValue) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x05}, std::byte{0x00}, std::byte{0x05}, std::byte{0xFF}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleCoil::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteSingleCoilTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x06}, std::byte{0x00}, std::byte{0x05}, std::byte{0xFF}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleCoil::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestWriteSingleCoilTest, DeserializeInvalidCoilValue) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x05}, std::byte{0x00}, std::byte{0x05}, std::byte{0x12}, std::byte{0x34}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleCoil::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidValue);
}

UTEST(RequestWriteSingleCoilTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x05},
        std::byte{0x00},
        std::byte{0x05},
        std::byte{0xFF},
        std::byte{0x00},
        std::byte{0x01}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteSingleCoil::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0x01});
}
