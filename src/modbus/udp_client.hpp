#pragma once

#include <userver/engine/io/socket.hpp>

#include <modbus/client_base.hpp>

namespace modbus {

class UdpClient final : public ClientBase {
public:
    UdpClient(std::uint8_t slave_id, userver::engine::io::Sockaddr endpoint, std::chrono::milliseconds timeout);

    userver::utils::expected<std::size_t, ClientError> SendRawRequest(
        std::span<const std::byte> request_pdu,
        std::span<std::byte> response_pdu_out
    ) override;

private:
    void DrainSocket();

    userver::engine::io::Socket socket_;
    userver::engine::io::Sockaddr endpoint_;
    std::chrono::milliseconds timeout_;
};

}  // namespace modbus
