#include <userver/utest/utest.hpp>

#include <modbus/coil.hpp>

UTEST(CoilTest, CoilToRaw) {
    EXPECT_EQ(modbus::CoilToRaw(modbus::Coil::kOn), 0xFF00);
    EXPECT_EQ(modbus::CoilToRaw(modbus::Coil::kOff), 0x0000);
}

UTEST(CoilTest, CoilFromRaw) {
    const auto coil_on = modbus::CoilFromRaw(0xFF00);
    ASSERT_TRUE(coil_on.has_value());
    EXPECT_EQ(*coil_on, modbus::Coil::kOn);

    const auto coil_off = modbus::CoilFromRaw(0x0000);
    ASSERT_TRUE(coil_off.has_value());
    EXPECT_EQ(*coil_off, modbus::Coil::kOff);

    const auto coil_invalid = modbus::CoilFromRaw(0xFFFF);
    EXPECT_FALSE(coil_invalid.has_value());
}
