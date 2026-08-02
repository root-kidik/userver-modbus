#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/utils.hpp>

UTEST(UtilsTest, WriteBeUint8) {
    std::vector<std::uint8_t> buffer;
    std::ignore = modbus::WriteBe(std::back_inserter(buffer), static_cast<std::uint8_t>(0x12));

    const std::vector<std::uint8_t> expected{0x12};
    EXPECT_EQ(buffer, expected);
}

UTEST(UtilsTest, WriteBeUint16) {
    std::vector<std::uint8_t> buffer;
    std::ignore = modbus::WriteBe(std::back_inserter(buffer), static_cast<std::uint16_t>(0x3456));

    const std::vector<std::uint8_t> expected{0x34, 0x56};
    EXPECT_EQ(buffer, expected);
}

UTEST(UtilsTest, WriteBeUint32) {
    std::vector<std::uint8_t> buffer;
    std::ignore = modbus::WriteBe(std::back_inserter(buffer), static_cast<std::uint32_t>(0x789ABCDE));

    const std::vector<std::uint8_t> expected{0x78, 0x9A, 0xBC, 0xDE};
    EXPECT_EQ(buffer, expected);
}

UTEST(UtilsTest, ReadBeUint8Success) {
    const std::vector<std::uint8_t> buffer{0x12};
    auto it = buffer.cbegin();

    const auto val = modbus::ReadBe<std::uint8_t>(it, buffer.cend());
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 0x12);
}

UTEST(UtilsTest, ReadBeUint16Success) {
    const std::vector<std::uint8_t> buffer{0x34, 0x56};
    auto it = buffer.cbegin();

    const auto val = modbus::ReadBe<std::uint16_t>(it, buffer.cend());
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 0x3456);
}

UTEST(UtilsTest, ReadBeUint32Success) {
    const std::vector<std::uint8_t> buffer{0x78, 0x9A, 0xBC, 0xDE};
    auto it = buffer.cbegin();

    const auto val = modbus::ReadBe<std::uint32_t>(it, buffer.cend());
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 0x789ABCDE);
}

UTEST(UtilsTest, ReadBeUint8BufferTooShort) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto val = modbus::ReadBe<std::uint8_t>(it, buffer.cend());
    ASSERT_FALSE(val.has_value());
    EXPECT_EQ(val.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(UtilsTest, ReadBeUint16BufferTooShort) {
    const std::vector<std::uint8_t> buffer{0x12};
    auto it = buffer.cbegin();

    const auto val = modbus::ReadBe<std::uint16_t>(it, buffer.cend());
    ASSERT_FALSE(val.has_value());
    EXPECT_EQ(val.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(UtilsTest, ReadBeUint32BufferTooShort) {
    const std::vector<std::uint8_t> buffer{0x12, 0x34, 0x56};
    auto it = buffer.cbegin();

    const auto val = modbus::ReadBe<std::uint32_t>(it, buffer.cend());
    ASSERT_FALSE(val.has_value());
    EXPECT_EQ(val.error(), modbus::ParseError::kBufferTooShort);
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
