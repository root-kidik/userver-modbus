#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/utils.hpp>

UTEST(UtilsTest, WriteBeUint8Success) {
    std::array<std::byte, 1> raw_buffer{};
    std::span<std::byte> buffer{raw_buffer};

    const auto res = modbus::WriteBe(buffer, static_cast<std::uint8_t>(0x12));

    ASSERT_TRUE(res.has_value());
    EXPECT_TRUE(buffer.empty());

    const std::array<std::byte, 1> expected{std::byte{0x12}};
    EXPECT_EQ(raw_buffer, expected);
}

UTEST(UtilsTest, WriteBeUint16Success) {
    std::array<std::byte, 2> raw_buffer{};
    std::span<std::byte> buffer{raw_buffer};

    const auto res = modbus::WriteBe(buffer, static_cast<std::uint16_t>(0x3456));

    ASSERT_TRUE(res.has_value());
    EXPECT_TRUE(buffer.empty());

    const std::array<std::byte, 2> expected{std::byte{0x34}, std::byte{0x56}};
    EXPECT_EQ(raw_buffer, expected);
}

UTEST(UtilsTest, WriteBeUint32Success) {
    std::array<std::byte, 4> raw_buffer{};
    std::span<std::byte> buffer{raw_buffer};

    const auto res = modbus::WriteBe(buffer, static_cast<std::uint32_t>(0x789ABCDE));

    ASSERT_TRUE(res.has_value());
    EXPECT_TRUE(buffer.empty());

    const std::array<std::byte, 4> expected{std::byte{0x78}, std::byte{0x9A}, std::byte{0xBC}, std::byte{0xDE}};
    EXPECT_EQ(raw_buffer, expected);
}

UTEST(UtilsTest, WriteBeBufferTooShort) {
    std::array<std::byte, 1> raw_buffer{};
    std::span<std::byte> buffer{raw_buffer};

    const auto res = modbus::WriteBe(buffer, static_cast<std::uint16_t>(0x1234));

    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(UtilsTest, ReadBeUint8Success) {
    const std::array<std::byte, 1> raw_buffer{std::byte{0x12}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto val = modbus::ReadBe<std::uint8_t>(buffer);
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 0x12);
    EXPECT_TRUE(buffer.empty());
}

UTEST(UtilsTest, ReadBeUint16Success) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x34}, std::byte{0x56}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto val = modbus::ReadBe<std::uint16_t>(buffer);
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 0x3456);
    EXPECT_TRUE(buffer.empty());
}

UTEST(UtilsTest, ReadBeUint32Success) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x78}, std::byte{0x9A}, std::byte{0xBC}, std::byte{0xDE}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto val = modbus::ReadBe<std::uint32_t>(buffer);
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 0x789ABCDE);
    EXPECT_TRUE(buffer.empty());
}

UTEST(UtilsTest, ReadBeUint8BufferTooShort) {
    std::span<const std::byte> buffer{};

    const auto val = modbus::ReadBe<std::uint8_t>(buffer);
    ASSERT_FALSE(val.has_value());
    EXPECT_EQ(val.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(UtilsTest, ReadBeUint16BufferTooShort) {
    const std::array<std::byte, 1> raw_buffer{std::byte{0x12}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto val = modbus::ReadBe<std::uint16_t>(buffer);
    ASSERT_FALSE(val.has_value());
    EXPECT_EQ(val.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(UtilsTest, ReadBeUint32BufferTooShort) {
    const std::array<std::byte, 3> raw_buffer{std::byte{0x12}, std::byte{0x34}, std::byte{0x56}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto val = modbus::ReadBe<std::uint32_t>(buffer);
    ASSERT_FALSE(val.has_value());
    EXPECT_EQ(val.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(UtilsTest, ReadBePreservesTailBuffer) {
    const std::array<std::byte, 3> raw_buffer{std::byte{0x34}, std::byte{0x56}, std::byte{0x78}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto val = modbus::ReadBe<std::uint16_t>(buffer);
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 0x3456);
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0x78});
}

UTEST(UtilsTest, WouldAddressOverflow) {
    EXPECT_FALSE(modbus::WouldAddressOverflow(0xFFFF, 1));
    EXPECT_TRUE(modbus::WouldAddressOverflow(0xFFFF, 2));
}

UTEST(UtilsTest, BitsToBytes) {
    EXPECT_EQ(modbus::BitsToBytes(0), 0);
    EXPECT_EQ(modbus::BitsToBytes(1), 1);
    EXPECT_EQ(modbus::BitsToBytes(7), 1);
    EXPECT_EQ(modbus::BitsToBytes(8), 1);
    EXPECT_EQ(modbus::BitsToBytes(9), 2);
}
