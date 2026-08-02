#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/response/read_discrete_inputs.hpp>

UTEST(ResponseReadDiscreteInputsTest, CreateSuccess) {
    const std::vector<modbus::DiscreteInput>
        inputs{modbus::DiscreteInput::kOn, modbus::DiscreteInput::kOff, modbus::DiscreteInput::kOn};
    const auto response = modbus::response::ReadDiscreteInputs::Create(inputs);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->GetByteCount(), 1);
    EXPECT_EQ(response->GetValues().size(), 3);
}

UTEST(ResponseReadDiscreteInputsTest, CreateMinQuantity) {
    const std::vector<modbus::DiscreteInput> inputs{modbus::DiscreteInput::kOn};
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
    const std::vector<modbus::DiscreteInput> inputs{};
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

UTEST(ResponseReadDiscreteInputsTest, Serialize) {
    const std::vector<modbus::DiscreteInput> inputs{
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

    std::vector<std::uint8_t> buffer;
    std::ignore = response->Serialize(std::back_inserter(buffer));

    const std::vector<std::uint8_t> expected{0x02, 0x02, 0x65, 0x01};
    EXPECT_EQ(buffer, expected);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeSuccess) {
    const std::vector<std::uint8_t> buffer{0x02, 0x02, 0x65, 0x01};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(it, buffer.cend(), 9);
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
    EXPECT_EQ(it, buffer.cend());
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeBufferTooShortFc) {
    const std::vector<std::uint8_t> buffer{};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeInvalidFunctionCode) {
    const std::vector<std::uint8_t> buffer{0x01, 0x01, 0x01};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidFunctionCode);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeBufferTooShortByteCount) {
    const std::vector<std::uint8_t> buffer{0x02};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeInvalidQuantityZero) {
    const std::vector<std::uint8_t> buffer{0x02, 0x00};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(it, buffer.cend(), 0);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeInvalidQuantityOverflow) {
    const std::vector<std::uint8_t> buffer{0x02, 0xFA};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(it, buffer.cend(), 2001);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidQuantity);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeInvalidByteCount) {
    const std::vector<std::uint8_t> buffer{0x02, 0x01, 0x65};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(it, buffer.cend(), 9);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kInvalidValue);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeBufferTooShortData) {
    const std::vector<std::uint8_t> buffer{0x02, 0x02, 0x65};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(it, buffer.cend(), 9);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kBufferTooShort);
}

UTEST(ResponseReadDiscreteInputsTest, DeserializeExtraDataAtEnd) {
    const std::vector<std::uint8_t> buffer{0x02, 0x01, 0x01, 0xFF};
    auto it = buffer.cbegin();

    const auto response = modbus::response::ReadDiscreteInputs::Deserialize(it, buffer.cend(), 1);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error(), modbus::ParseError::kExtraDataAtEnd);
}
