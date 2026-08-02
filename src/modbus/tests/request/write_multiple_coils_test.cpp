#include <array>
#include <cstddef>
#include <span>
#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/request/write_multiple_coils.hpp>

UTEST(RequestWriteMultipleCoilsTest, CreateSuccess) {
    const std::vector<modbus::Coil> coils{
        modbus::Coil::kOn,
        modbus::Coil::kOff,
        modbus::Coil::kOn,
        modbus::Coil::kOff,
        modbus::Coil::kOff,
        modbus::Coil::kOn,
        modbus::Coil::kOn,
        modbus::Coil::kOff,
        modbus::Coil::kOn,
        modbus::Coil::kOff
    };
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0013, coils);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0013);
    EXPECT_EQ(request->GetQuantity(), 10);
    EXPECT_EQ(request->GetByteCount(), 2);
    EXPECT_EQ(request->GetValues().size(), 10);
}

UTEST(RequestWriteMultipleCoilsTest, CreateMinQuantity) {
    const std::vector<modbus::Coil> coils{modbus::Coil::kOn};
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0000, coils);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestWriteMultipleCoilsTest, CreateMaxQuantity) {
    const std::vector<modbus::Coil> coils(1968, modbus::Coil::kOn);
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0000, coils);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1968);
}

UTEST(RequestWriteMultipleCoilsTest, CreateInvalidQuantityZero) {
    const std::vector<modbus::Coil> coils{};
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0000, coils);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleCoilsTest, CreateInvalidQuantityOverflow) {
    const std::vector<modbus::Coil> coils(1969, modbus::Coil::kOn);
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0000, coils);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleCoilsTest, CreateAddressOverflow) {
    const std::vector<modbus::Coil> coils(2, modbus::Coil::kOn);
    const auto request = modbus::request::WriteMultipleCoils::Create(0xFFFF, coils);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestWriteMultipleCoilsTest, CreateInvalidValue) {
    const std::vector<modbus::Coil> coils{static_cast<modbus::Coil>(0x1234)};
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0000, coils);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidValue);
}

UTEST(RequestWriteMultipleCoilsTest, SerializeSuccess) {
    const std::vector<modbus::Coil> coils{
        modbus::Coil::kOn,
        modbus::Coil::kOff,
        modbus::Coil::kOn,
        modbus::Coil::kOff,
        modbus::Coil::kOff,
        modbus::Coil::kOn,
        modbus::Coil::kOn,
        modbus::Coil::kOff,
        modbus::Coil::kOn,
        modbus::Coil::kOff
    };
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0013, coils);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 8> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 8> expected{
        std::byte{0x0F},
        std::byte{0x00},
        std::byte{0x13},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x02},
        std::byte{0x65},
        std::byte{0x01}
    };
    EXPECT_EQ(out_buffer, expected);
}

UTEST(RequestWriteMultipleCoilsTest, SerializeBufferTooShort) {
    const std::vector<modbus::Coil> coils{modbus::Coil::kOn};
    const auto request = modbus::request::WriteMultipleCoils::Create(0x0000, coils);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 6> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeSuccess) {
    const std::array<std::byte, 8> raw_buffer{
        std::byte{0x0F},
        std::byte{0x00},
        std::byte{0x13},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x02},
        std::byte{0x65},
        std::byte{0x01}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0013);
    EXPECT_EQ(request->GetQuantity(), 10);
    EXPECT_EQ(request->GetByteCount(), 2);

    const auto coils = request->GetValues();
    ASSERT_EQ(coils.size(), 10);
    EXPECT_EQ(coils[0], modbus::Coil::kOn);
    EXPECT_EQ(coils[1], modbus::Coil::kOff);
    EXPECT_EQ(coils[2], modbus::Coil::kOn);
    EXPECT_EQ(coils[3], modbus::Coil::kOff);
    EXPECT_EQ(coils[4], modbus::Coil::kOff);
    EXPECT_EQ(coils[5], modbus::Coil::kOn);
    EXPECT_EQ(coils[6], modbus::Coil::kOn);
    EXPECT_EQ(coils[7], modbus::Coil::kOff);
    EXPECT_EQ(coils[8], modbus::Coil::kOn);
    EXPECT_EQ(coils[9], modbus::Coil::kOff);
    EXPECT_TRUE(buffer.empty());
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x0F}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeBufferTooShortQuantity) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x0F}, std::byte{0x00}, std::byte{0x13}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeBufferTooShortByteCount) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x0F}, std::byte{0x00}, std::byte{0x13}, std::byte{0x00}, std::byte{0x0A}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeBufferTooShortDataBytes) {
    const std::array<std::byte, 7> raw_buffer{
        std::byte{0x0F},
        std::byte{0x00},
        std::byte{0x13},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x02},
        std::byte{0x65}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 8> raw_buffer{
        std::byte{0x10},
        std::byte{0x00},
        std::byte{0x13},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x02},
        std::byte{0x65},
        std::byte{0x01}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeInvalidQuantityUnderflow) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x0F},
        std::byte{0x00},
        std::byte{0x13},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeInvalidQuantityOverflow) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x0F},
        std::byte{0x00},
        std::byte{0x13},
        std::byte{0x07},
        std::byte{0xB1},
        std::byte{0xF6}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeAddressOverflow) {
    const std::array<std::byte, 7> raw_buffer{
        std::byte{0x0F},
        std::byte{0xFF},
        std::byte{0xFF},
        std::byte{0x00},
        std::byte{0x02},
        std::byte{0x01},
        std::byte{0x03}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializeInvalidByteCount) {
    const std::array<std::byte, 9> raw_buffer{
        std::byte{0x0F},
        std::byte{0x00},
        std::byte{0x13},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x03},
        std::byte{0x65},
        std::byte{0x01},
        std::byte{0x00}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleCoilsTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 9> raw_buffer{
        std::byte{0x0F},
        std::byte{0x00},
        std::byte{0x13},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x02},
        std::byte{0x65},
        std::byte{0x01},
        std::byte{0xFF}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleCoils::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0xFF});
}
