#include <array>
#include <chrono>

#include <userver/engine/async.hpp>
#include <userver/engine/io/socket.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/utest/utest.hpp>

#include <modbus/rtu_frame.hpp>
#include <modbus/udp_client.hpp>

namespace {

struct UdpServer {
    UdpServer() : socket{userver::engine::io::AddrDomain::kInet, userver::engine::io::SocketType::kDgram} {
        socket.Bind(userver::engine::io::Sockaddr::MakeIPv4LoopbackAddress());
    }

    userver::engine::io::Socket socket;
};

struct UdpClientTest : testing::Test {
    UdpClientTest() : client{slave_id, metrics, server.socket.Getsockname(), timeout} {}

    std::uint8_t slave_id{1};

    std::chrono::milliseconds timeout{200};

    UdpServer server;

    modbus::ClientMetrics metrics;
    modbus::UdpClient client;
};

}  // namespace

UTEST_F(UdpClientTest, SuccessTransaction) {
    const std::array<std::byte, 3> response_pdu{std::byte{0x12}, std::byte{0x21}, std::byte{0x42}};

    auto server_task = userver::engine::AsyncNoTracing([this, &response_pdu]() {
        const auto deadline = userver::engine::Deadline::FromDuration(std::chrono::seconds{1});

        std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> request_buffer;

        const auto [received_bytes, src_addr] =
            server.socket.RecvSomeFrom(request_buffer.data(), request_buffer.size(), deadline);

        std::span<const std::byte> request_span{request_buffer.data(), received_bytes};

        const auto request_adu = modbus::RtuFrame::Deserialize(request_span);
        ASSERT_TRUE(request_adu.has_value());
        EXPECT_EQ(request_adu->GetSlaveId(), slave_id);

        const auto response_adu = modbus::RtuFrame::Create(slave_id, response_pdu).value();

        std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> response_buffer;
        ASSERT_TRUE(response_adu.Serialize(response_buffer).has_value());

        EXPECT_EQ(
            server.socket.SendAllTo(src_addr, response_buffer.data(), response_adu.GetFrameSize(), deadline),
            response_adu.GetFrameSize()
        );
    });

    const std::array<std::byte, 4> request_pdu{std::byte{0x03}, std::byte{0x00}, std::byte{0x00}, std::byte{0x01}};
    std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> response_pdu_out;

    const auto result = client.SendRawRequest(request_pdu, response_pdu_out);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, response_pdu.size());
    EXPECT_TRUE(std::ranges::equal(response_pdu, std::span{response_pdu_out}.first(*result)));

    server_task.Get();
}

UTEST_F(UdpClientTest, Timeout) {
    const std::array<std::byte, 4> request_pdu{std::byte{0x03}, std::byte{0x00}, std::byte{0x00}, std::byte{0x01}};
    std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> response_pdu_out;

    const auto result = client.SendRawRequest(request_pdu, response_pdu_out);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ClientError::kTimeout);
}

UTEST_F(UdpClientTest, InvalidSlaveId) {
    const std::array<std::byte, 3> response_pdu{std::byte{0x12}, std::byte{0x21}, std::byte{0x42}};

    auto server_task = userver::engine::AsyncNoTracing([this, &response_pdu]() {
        const auto deadline = userver::engine::Deadline::FromDuration(std::chrono::seconds{1});

        std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> request_buffer;

        const auto [received_bytes, src_addr] =
            server.socket.RecvSomeFrom(request_buffer.data(), request_buffer.size(), deadline);

        const std::uint8_t wrong_slave_id = slave_id + 1;
        const auto response_adu = modbus::RtuFrame::Create(wrong_slave_id, response_pdu).value();

        std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> response_buffer;
        ASSERT_TRUE(response_adu.Serialize(response_buffer).has_value());

        EXPECT_EQ(
            server.socket.SendAllTo(src_addr, response_buffer.data(), response_adu.GetFrameSize(), deadline),
            response_adu.GetFrameSize()
        );
    });

    const std::array<std::byte, 4> request_pdu{std::byte{0x03}, std::byte{0x00}, std::byte{0x00}, std::byte{0x01}};
    std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> response_pdu_out;

    const auto result = client.SendRawRequest(request_pdu, response_pdu_out);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ClientError::kInvalidResponse);

    server_task.Get();
}

UTEST_F(UdpClientTest, InvalidCrc) {
    auto server_task = userver::engine::AsyncNoTracing([this]() {
        const auto deadline = userver::engine::Deadline::FromDuration(std::chrono::seconds(1));

        std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> request_buffer;

        const auto [received_bytes, src_addr] =
            server.socket.RecvSomeFrom(request_buffer.data(), request_buffer.size(), deadline);

        const std::array<std::byte, 5>
            corrupted_adu{std::byte{slave_id}, std::byte{0x03}, std::byte{0x01}, std::byte{0xFF}, std::byte{0xFF}};

        EXPECT_EQ(
            server.socket.SendAllTo(src_addr, corrupted_adu.data(), corrupted_adu.size(), deadline),
            corrupted_adu.size()
        );
    });

    const std::array<std::byte, 4> request_pdu{std::byte{0x03}, std::byte{0x00}, std::byte{0x00}, std::byte{0x01}};
    std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> response_pdu_out;

    const auto result = client.SendRawRequest(request_pdu, response_pdu_out);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ClientError::kInvalidResponse);

    server_task.Get();
}

UTEST_F(UdpClientTest, BufferTooSmall) {
    const std::array<std::byte, 3> response_pdu{std::byte{0x12}, std::byte{0x21}, std::byte{0x42}};

    auto server_task = userver::engine::AsyncNoTracing([this, &response_pdu]() {
        const auto deadline = userver::engine::Deadline::FromDuration(std::chrono::seconds(1));

        std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> request_buffer;
        const auto [received_bytes, src_addr] =
            server.socket.RecvSomeFrom(request_buffer.data(), request_buffer.size(), deadline);

        const auto response_adu = modbus::RtuFrame::Create(slave_id, response_pdu).value();

        std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> response_buffer;
        ASSERT_TRUE(response_adu.Serialize(response_buffer).has_value());

        EXPECT_EQ(
            server.socket.SendAllTo(src_addr, response_buffer.data(), response_adu.GetFrameSize(), deadline),
            response_adu.GetFrameSize()
        );
    });

    const std::array<std::byte, 4> request_pdu{std::byte{0x03}, std::byte{0x00}, std::byte{0x00}, std::byte{0x01}};
    std::array<std::byte, 2> too_small_buffer;

    const auto result = client.SendRawRequest(request_pdu, too_small_buffer);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), modbus::ClientError::kBufferTooSmall);

    server_task.Get();
}

UTEST_F(UdpClientTest, DrainStalePackets) {
    const std::array<std::byte, 3> stale_pdu{std::byte{0x99}, std::byte{0x99}, std::byte{0x99}};
    const std::array<std::byte, 3> fresh_pdu{std::byte{0x12}, std::byte{0x21}, std::byte{0x42}};

    auto server_task = userver::engine::AsyncNoTracing([this, &stale_pdu, &fresh_pdu]() {
        const auto deadline = userver::engine::Deadline::FromDuration(std::chrono::seconds(2));

        std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> request_buffer;

        const auto
            [bytes1, src_addr1] = server.socket.RecvSomeFrom(request_buffer.data(), request_buffer.size(), deadline);

        userver::engine::SleepFor(timeout + std::chrono::milliseconds{50});

        const auto stale_adu = modbus::RtuFrame::Create(slave_id, stale_pdu).value();
        std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> stale_buffer;
        ASSERT_TRUE(stale_adu.Serialize(stale_buffer).has_value());
        EXPECT_EQ(
            server.socket.SendAllTo(src_addr1, stale_buffer.data(), stale_adu.GetFrameSize(), deadline),
            stale_adu.GetFrameSize()
        );

        const auto
            [bytes2, src_addr2] = server.socket.RecvSomeFrom(request_buffer.data(), request_buffer.size(), deadline);

        const auto fresh_adu = modbus::RtuFrame::Create(slave_id, fresh_pdu).value();
        std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> fresh_buffer;
        ASSERT_TRUE(fresh_adu.Serialize(fresh_buffer).has_value());
        EXPECT_EQ(
            server.socket.SendAllTo(src_addr2, fresh_buffer.data(), fresh_adu.GetFrameSize(), deadline),
            fresh_adu.GetFrameSize()
        );
    });

    const std::array<std::byte, 4> request_pdu{std::byte{0x03}, std::byte{0x00}, std::byte{0x00}, std::byte{0x01}};
    std::array<std::byte, modbus::RtuFrame::kMaxFrameSize> response_pdu_out;

    const auto result1 = client.SendRawRequest(request_pdu, response_pdu_out);
    ASSERT_FALSE(result1.has_value());
    EXPECT_EQ(result1.error(), modbus::ClientError::kTimeout);

    userver::engine::SleepFor(std::chrono::milliseconds{100});

    const auto result2 = client.SendRawRequest(request_pdu, response_pdu_out);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, fresh_pdu.size());
    EXPECT_TRUE(std::ranges::equal(fresh_pdu, std::span{response_pdu_out}.first(*result2)));

    server_task.Get();
}
