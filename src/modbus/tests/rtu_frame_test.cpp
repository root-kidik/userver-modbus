#include <userver/utest/utest.hpp>

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
#include <vector>

#include <modbus/rtu_frame.hpp>

namespace {

std::vector<std::byte> MakeBytes(std::initializer_list<std::uint8_t> bytes) {
    std::vector<std::byte> result;
    result.reserve(bytes.size());
    for (const auto b : bytes) {
        result.push_back(static_cast<std::byte>(b));
    }
    return result;
}

bool SpanEq(std::span<const std::byte> actual, std::initializer_list<std::uint8_t> expected) {
    if (actual.size() != expected.size()) {
        return false;
    }
    auto it = expected.begin();
    for (const auto& byte : actual) {
        if (static_cast<std::uint8_t>(byte) != *it++) {
            return false;
        }
    }
    return true;
}

}  // namespace

UTEST(RtuFrameTest, CreateSuccess) {
    auto pdu = MakeBytes({0x03, 0x00, 0x6B, 0x00, 0x03});
    const auto frame_result = modbus::RtuFrame::Create(0x11, std::move(pdu));

    ASSERT_TRUE(frame_result.has_value());
    EXPECT_EQ(frame_result->GetSlaveId(), 0x11);
    EXPECT_TRUE(SpanEq(frame_result->GetPdu(), {0x03, 0x00, 0x6B, 0x00, 0x03}));
}

UTEST(RtuFrameTest, CreateFailsWhenPduTooLarge) {
    auto huge_pdu = std::vector<std::byte>(modbus::RtuFrame::kMaxPduSize + 1, std::byte{0});
    const auto frame_result = modbus::RtuFrame::Create(0x01, std::move(huge_pdu));

    ASSERT_FALSE(frame_result.has_value());
    EXPECT_EQ(frame_result.error(), modbus::RtuError::kPduTooLarge);
}

UTEST(RtuFrameTest, Serialize) {
    auto pdu = MakeBytes({0x03, 0x00, 0x6B, 0x00, 0x03});
    const auto frame = modbus::RtuFrame::Create(0x11, std::move(pdu)).value();

    std::vector<std::byte> serialized_data;
    std::ignore = frame.Serialize(std::back_inserter(serialized_data));

    const auto expected_data = MakeBytes({0x11, 0x03, 0x00, 0x6B, 0x00, 0x03, 0x76, 0x87});

    EXPECT_EQ(serialized_data, expected_data);
}

UTEST(RtuFrameTest, DeserializeSuccess) {
    const auto buffer = MakeBytes({0x11, 0x03, 0x00, 0x6B, 0x00, 0x03, 0x76, 0x87});

    auto it = buffer.cbegin();
    const auto frame_result = modbus::RtuFrame::Deserialize(it, buffer.cend());

    ASSERT_TRUE(frame_result.has_value());
    EXPECT_EQ(frame_result->GetSlaveId(), 0x11);
    EXPECT_TRUE(SpanEq(frame_result->GetPdu(), {0x03, 0x00, 0x6B, 0x00, 0x03}));

    EXPECT_EQ(it, buffer.cend());
}

UTEST(RtuFrameTest, DeserializeFailsBufferTooShort) {
    const auto buffer = MakeBytes({0x11, 0x03});

    auto it = buffer.cbegin();
    const auto frame_result = modbus::RtuFrame::Deserialize(it, buffer.cend());

    ASSERT_FALSE(frame_result.has_value());
    EXPECT_EQ(frame_result.error(), modbus::RtuError::kBufferTooShort);
}

UTEST(RtuFrameTest, DeserializeFailsPduTooLarge) {
    const auto buffer = std::vector<
        std::byte>(modbus::RtuFrame::kMinFrameSize + modbus::RtuFrame::kMaxPduSize + 1, std::byte{0});

    auto it = buffer.cbegin();
    const auto frame_result = modbus::RtuFrame::Deserialize(it, buffer.cend());

    ASSERT_FALSE(frame_result.has_value());
    EXPECT_EQ(frame_result.error(), modbus::RtuError::kPduTooLarge);
}

UTEST(RtuFrameTest, DeserializeFailsInvalidCrc) {
    const auto buffer = MakeBytes({0x11, 0x03, 0x00, 0x6B, 0x00, 0x03, 0x76, 0xFF});

    auto it = buffer.cbegin();
    const auto frame_result = modbus::RtuFrame::Deserialize(it, buffer.cend());

    ASSERT_FALSE(frame_result.has_value());
    EXPECT_EQ(frame_result.error(), modbus::RtuError::kInvalidCrc);
}

UTEST(RtuFrameTest, RoundtripWithEmptyPdu) {
    std::vector<std::byte> empty_pdu;
    const auto frame = modbus::RtuFrame::Create(0x01, std::move(empty_pdu)).value();

    std::vector<std::byte> serialized_data;
    std::ignore = frame.Serialize(std::back_inserter(serialized_data));

    EXPECT_EQ(serialized_data.size(), 3);

    auto it = serialized_data.cbegin();
    const auto deserialized_frame = modbus::RtuFrame::Deserialize(it, serialized_data.cend());

    ASSERT_TRUE(deserialized_frame.has_value());
    EXPECT_EQ(deserialized_frame->GetSlaveId(), 0x01);
    EXPECT_TRUE(deserialized_frame->GetPdu().empty());
}
