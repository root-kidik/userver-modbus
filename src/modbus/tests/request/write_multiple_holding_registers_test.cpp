#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/request/write_multiple_holding_registers.hpp>

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateSuccess) {
    const std::vector<std::uint16_t> registers{0x000A, 0x0102};
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0001, registers);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0001);
    EXPECT_EQ(request->GetQuantity(), 2);
    EXPECT_EQ(request->GetByteCount(), 4);
    EXPECT_EQ(request->GetValues().size(), 2);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateMinQuantity) {
    const std::vector<std::uint16_t> registers{0x1234};
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0000, registers);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 1);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateMaxQuantity) {
    const std::vector<std::uint16_t> registers(123, 0x1234);
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0000, registers);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetQuantity(), 123);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateInvalidQuantityZero) {
    const std::vector<std::uint16_t> registers{};
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0001, registers);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateInvalidQuantityOverflow) {
    const std::vector<std::uint16_t> registers(124, 0x1234);
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0001, registers);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, CreateAddressOverflow) {
    const std::vector<std::uint16_t> registers(2, 0x1234);
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0xFFFF, registers);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, SerializeSuccess) {
    const std::vector<std::uint16_t> registers{0x000A, 0x0102};
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0001, registers);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 10> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 10> expected{
        std::byte{0x10},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x02},
        std::byte{0x04},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x01},
        std::byte{0x02}
    };
    EXPECT_EQ(out_buffer, expected);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, SerializeBufferTooShort) {
    const std::vector<std::uint16_t> registers{0x000A};
    const auto request = modbus::request::WriteMultipleHoldingRegisters::Create(0x0001, registers);
    ASSERT_TRUE(request.has_value());

    std::array<std::byte, 6> out_buffer{};
    const auto result = request->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeSuccess) {
    const std::array<std::byte, 10> raw_buffer{
        std::byte{0x10},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x02},
        std::byte{0x04},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x01},
        std::byte{0x02}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->GetAddress(), 0x0001);
    EXPECT_EQ(request->GetQuantity(), 2);
    EXPECT_EQ(request->GetByteCount(), 4);

    const auto registers = request->GetValues();
    ASSERT_EQ(registers.size(), 2);
    EXPECT_EQ(registers[0], 0x000A);
    EXPECT_EQ(registers[1], 0x0102);
    EXPECT_TRUE(buffer.empty());
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortAddress) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x10}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortQuantity) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x10}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortByteCount) {
    const std::array<std::byte, 5>
        raw_buffer{std::byte{0x10}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x02}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeBufferTooShortDataBytes) {
    const std::array<std::byte, 9> raw_buffer{
        std::byte{0x10},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x02},
        std::byte{0x04},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x01}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 10> raw_buffer{
        std::byte{0x0F},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x02},
        std::byte{0x04},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x01},
        std::byte{0x02}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeInvalidQuantityUnderflow) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x10},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeInvalidQuantityOverflow) {
    const std::array<std::byte, 6> raw_buffer{
        std::byte{0x10},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x7C},
        std::byte{0xF8}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeAddressOverflow) {
    const std::array<std::byte, 10> raw_buffer{
        std::byte{0x10},
        std::byte{0xFF},
        std::byte{0xFF},
        std::byte{0x00},
        std::byte{0x02},
        std::byte{0x04},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x02}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kAddressOverflow);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializeInvalidByteCount) {
    const std::array<std::byte, 10> raw_buffer{
        std::byte{0x10},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x02},
        std::byte{0x05},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x01},
        std::byte{0x02}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_FALSE(request.has_value());
    EXPECT_EQ(request.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(RequestWriteMultipleHoldingRegistersTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 11> raw_buffer{
        std::byte{0x10},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x02},
        std::byte{0x04},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0xBB}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto request = modbus::request::WriteMultipleHoldingRegisters::Deserialize(buffer);
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0xBB});
}
