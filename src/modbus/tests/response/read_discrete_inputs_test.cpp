#include <array>
#include <cstddef>
#include <span>
#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/parse_error.hpp>
#include <modbus/response/read_discrete_inputs.hpp>

UTEST(ResponseReadDiscreteInputsTest, CreateSuccess) {
    const std::array inputs{modbus::DiscreteInput::kOn, modbus::DiscreteInput::kOff, modbus::DiscreteInput::kOn};
    const auto response = modbus::response::ReadDiscreteInputs::Create(inputs);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 1);
    EXPECT_EQ(response->GetValues().size(), 3);
}

UTEST(ResponseReadDiscreteInputsTest, CreateMinQuantity) {
    const std::array inputs{modbus::DiscreteInput::kOn};
    const auto response = modbus::response::ReadDiscreteInputs::Create(inputs);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetValues().size(), 1);
}

UTEST(ResponseReadDiscreteInputsTest, CreateMaxQuantity) {
    const std::vector<modbus::DiscreteInput> inputs(2000, modbus::DiscreteInput::kOn);
    const auto response = modbus::response::ReadDiscreteInputs::Create(inputs);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetValues().size(), 2000);
}

UTEST(ResponseReadDiscreteInputsTest, CreateInvalidQuantityZero) {
    const std::array<modbus::DiscreteInput, 0> inputs{};
    const auto response = modbus::response::ReadDiscreteInputs::Create(inputs);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadDiscreteInputsTest, CreateInvalidQuantityOverflow) {
    const std::vector<modbus::DiscreteInput> inputs(2001, modbus::DiscreteInput::kOn);
    const auto response = modbus::response::ReadDiscreteInputs::Create(inputs);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadDiscreteInputsTest, SerializeSuccess) {
    const std::array inputs{
        modbus::DiscreteInput::kOn,
        modbus::DiscreteInput::kOff,
        modbus::DiscreteInput::kOn,
        modbus::DiscreteInput::kOff,
        modbus::DiscreteInput::kOff,
        modbus::DiscreteInput::kOn,
        modbus::DiscreteInput::kOn,
        modbus::DiscreteInput::kOff,
        modbus::DiscreteInput::kOn
    };
    const auto response = modbus::response::ReadDiscreteInputs::Create(inputs);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 4> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 4> expected{std::byte{0x02}, std::byte{0x02}, std::byte{0x65}, std::byte{0x01}};
    EXPECT_EQ(out_buffer, expected);
}

UTEST(ResponseReadDiscreteInputsTest, SerializeBufferTooShort) {
    const std::array inputs{modbus::DiscreteInput::kOn, modbus::DiscreteInput::kOff, modbus::DiscreteInput::kOn};
    const auto response = modbus::response::ReadDiscreteInputs::Create(inputs);
    ASSERT_TRUE(response.has_value());

    std::array<std::byte, 2> out_buffer{};
    const auto result = response->Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeSuccess) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x02}, std::byte{0x02}, std::byte{0x65}, std::byte{0x01}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(buffer, 9);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 2);

    const auto values = response->GetValues();
    ASSERT_EQ(values.size(), 9);
    EXPECT_EQ(values[0], modbus::DiscreteInput::kOn);
    EXPECT_EQ(values[1], modbus::DiscreteInput::kOff);
    EXPECT_EQ(values[2], modbus::DiscreteInput::kOn);
    EXPECT_EQ(values[3], modbus::DiscreteInput::kOff);
    EXPECT_EQ(values[4], modbus::DiscreteInput::kOff);
    EXPECT_EQ(values[5], modbus::DiscreteInput::kOn);
    EXPECT_EQ(values[6], modbus::DiscreteInput::kOn);
    EXPECT_EQ(values[7], modbus::DiscreteInput::kOff);
    EXPECT_EQ(values[8], modbus::DiscreteInput::kOn);
    EXPECT_TRUE(buffer.empty());
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeBufferTooShortFc) {
    std::span<const std::byte> buffer{};

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeBufferTooShortByteCount) {
    const std::array<std::byte, 1> raw_buffer{std::byte{0x02}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeBufferTooShortData) {
    const std::array<std::byte, 3> raw_buffer{std::byte{0x02}, std::byte{0x02}, std::byte{0x65}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(buffer, 9);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeInvalidFunctionCode) {
    const std::array<std::byte, 3> raw_buffer{std::byte{0x01}, std::byte{0x01}, std::byte{0x01}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(buffer, 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeInvalidQuantityZero) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x02}, std::byte{0x00}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(buffer, 0);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeInvalidQuantityOverflow) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x02}, std::byte{0xFA}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(buffer, 2001);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeInvalidByteCount) {
    const std::array<std::byte, 3> raw_buffer{std::byte{0x02}, std::byte{0x01}, std::byte{0x65}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(buffer, 9);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializePreservesTailBuffer) {
    const std::array<std::byte, 4> raw_buffer{std::byte{0x02}, std::byte{0x01}, std::byte{0x01}, std::byte{0xFF}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(buffer, 1);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[0], std::byte{0xFF});
}
