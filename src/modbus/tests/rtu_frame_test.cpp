#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/rtu_frame.hpp>

UTEST(RtuFrameTest, CreateSuccess) {
    const std::array<std::byte, 5>
        pdu{std::byte{0x03}, std::byte{0x00}, std::byte{0x6B}, std::byte{0x00}, std::byte{0x03}};
    const auto frame_result = modbus::RtuFrame::Create(0x11, pdu);

    ASSERT_TRUE(frame_result.has_value());
    EXPECT_EQ(frame_result->GetSlaveId(), 0x11);
    EXPECT_TRUE(std::ranges::equal(frame_result->GetPdu(), pdu));
    EXPECT_EQ(frame_result->GetFrameSize(), 8);
}

UTEST(RtuFrameTest, CreateFailsWhenPduTooLarge) {
    const std::vector<std::byte> huge_pdu(modbus::kMaxPduSize + 1, std::byte{0});
    const auto frame_result = modbus::RtuFrame::Create(0x01, huge_pdu);

    ASSERT_FALSE(frame_result.has_value());
    EXPECT_EQ(frame_result.error(), modbus::RtuError::kPduTooLarge);
}

UTEST(RtuFrameTest, SerializeSuccess) {
    const std::array<std::byte, 5>
        pdu{std::byte{0x03}, std::byte{0x00}, std::byte{0x6B}, std::byte{0x00}, std::byte{0x03}};
    const auto frame = modbus::RtuFrame::Create(0x11, pdu).value();

    std::array<std::byte, 8> out_buffer{};
    const auto result = frame.Serialize(out_buffer);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());

    const std::array<std::byte, 8> expected{
        std::byte{0x11},
        std::byte{0x03},
        std::byte{0x00},
        std::byte{0x6B},
        std::byte{0x00},
        std::byte{0x03},
        std::byte{0x76},
        std::byte{0x87}
    };
    EXPECT_EQ(out_buffer, expected);
}

UTEST(RtuFrameTest, SerializeBufferTooShort) {
    const std::array<std::byte, 5>
        pdu{std::byte{0x03}, std::byte{0x00}, std::byte{0x6B}, std::byte{0x00}, std::byte{0x03}};
    const auto frame = modbus::RtuFrame::Create(0x11, pdu).value();

    std::array<std::byte, 7> out_buffer{};
    const auto result = frame.Serialize(out_buffer);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::RtuError::kBufferTooShort);
}

UTEST(RtuFrameTest, DeserializeSuccess) {
    const std::array<std::byte, 8> raw_buffer{
        std::byte{0x11},
        std::byte{0x03},
        std::byte{0x00},
        std::byte{0x6B},
        std::byte{0x00},
        std::byte{0x03},
        std::byte{0x76},
        std::byte{0x87}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto frame_result = modbus::RtuFrame::Deserialize(buffer);

    ASSERT_TRUE(frame_result.has_value());
    EXPECT_EQ(frame_result->GetSlaveId(), 0x11);

    const std::array<std::byte, 5>
        expected_pdu{std::byte{0x03}, std::byte{0x00}, std::byte{0x6B}, std::byte{0x00}, std::byte{0x03}};
    EXPECT_TRUE(std::ranges::equal(frame_result->GetPdu(), expected_pdu));
    EXPECT_TRUE(buffer.empty());
}

UTEST(RtuFrameTest, DeserializeFailsBufferTooShort) {
    const std::array<std::byte, 2> raw_buffer{std::byte{0x11}, std::byte{0x03}};
    std::span<const std::byte> buffer{raw_buffer};

    const auto frame_result = modbus::RtuFrame::Deserialize(buffer);

    ASSERT_FALSE(frame_result.has_value());
    EXPECT_EQ(frame_result.error(), modbus::RtuError::kBufferTooShort);
}

UTEST(RtuFrameTest, DeserializeFailsPduTooLarge) {
    const std::vector<std::byte> raw_buffer(modbus::RtuFrame::kMinFrameSize + modbus::kMaxPduSize + 1, std::byte{0});
    std::span<const std::byte> buffer{raw_buffer};

    const auto frame_result = modbus::RtuFrame::Deserialize(buffer);

    ASSERT_FALSE(frame_result.has_value());
    EXPECT_EQ(frame_result.error(), modbus::RtuError::kPduTooLarge);
}

UTEST(RtuFrameTest, DeserializeFailsInvalidCrc) {
    const std::array<std::byte, 8> raw_buffer{
        std::byte{0x11},
        std::byte{0x03},
        std::byte{0x00},
        std::byte{0x6B},
        std::byte{0x00},
        std::byte{0x03},
        std::byte{0x76},
        std::byte{0xFF}
    };
    std::span<const std::byte> buffer{raw_buffer};

    const auto frame_result = modbus::RtuFrame::Deserialize(buffer);

    ASSERT_FALSE(frame_result.has_value());
    EXPECT_EQ(frame_result.error(), modbus::RtuError::kInvalidCrc);
}

UTEST(RtuFrameTest, RoundtripWithEmptyPdu) {
    const std::array<std::byte, 0> empty_pdu{};
    const auto frame = modbus::RtuFrame::Create(0x01, empty_pdu).value();

    std::array<std::byte, 3> serialized_data{};
    const auto ser_result = frame.Serialize(serialized_data);
    ASSERT_TRUE(ser_result.has_value());
    EXPECT_TRUE(ser_result->empty());

    std::span<const std::byte> buffer{serialized_data};
    const auto deserialized_frame = modbus::RtuFrame::Deserialize(buffer);

    ASSERT_TRUE(deserialized_frame.has_value());
    EXPECT_EQ(deserialized_frame->GetSlaveId(), 0x01);
    EXPECT_TRUE(deserialized_frame->GetPdu().empty());
    EXPECT_TRUE(buffer.empty());
}
