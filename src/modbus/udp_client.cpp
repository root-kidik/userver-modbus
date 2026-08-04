#include <modbus/udp_client.hpp>

#include <modbus/rtu_frame.hpp>

namespace modbus {

UdpClient::UdpClient(
    std::uint8_t slave_id,
    ClientMetrics& metrics,
    userver::engine::io::Sockaddr endpoint,
    std::chrono::milliseconds timeout
)
    : ClientBase{slave_id, metrics},
      socket_{endpoint.Domain(), userver::engine::io::SocketType::kDgram},
      endpoint_{endpoint},
      timeout_{timeout} {}

userver::utils::expected<std::size_t, ClientError> UdpClient::SendRawRequest(
    std::span<const std::byte> request_pdu,
    std::span<std::byte> response_pdu_out
) {
    DrainSocket();

    const auto request_adu = RtuFrame::Create(GetSlaveId(), request_pdu);
    if (!request_adu.has_value()) {
        return userver::utils::unexpected{ClientError::kInvalidRequest};
    }

    std::array<std::byte, RtuFrame::kMaxFrameSize> request_buffer;
    if (const auto result = request_adu->Serialize(request_buffer); !result.has_value()) {
        return userver::utils::unexpected{ClientError::kInvalidRequest};
    }

    const auto adu_size = request_adu->GetFrameSize();

    const auto deadline = userver::engine::Deadline::FromDuration(timeout_);

    std::array<std::byte, RtuFrame::kMaxFrameSize> response_buffer;
    std::size_t received_bytes = 0;

    try {
        if (socket_.SendAllTo(endpoint_, request_buffer.data(), adu_size, deadline) != adu_size) {
            return userver::utils::unexpected{ClientError::kTransportError};
        }

        received_bytes = socket_.RecvSome(response_buffer.data(), response_buffer.size(), deadline);
    } catch (const userver::engine::io::IoTimeout&) {
        return userver::utils::unexpected{ClientError::kTimeout};
    } catch (const userver::engine::io::IoException&) {
        return userver::utils::unexpected{ClientError::kTransportError};
    }

    std::span<const std::byte> response_span{response_buffer.data(), received_bytes};

    const auto response_adu = RtuFrame::Deserialize(response_span);
    if (!response_adu.has_value()) {
        return userver::utils::unexpected{ClientError::kInvalidResponse};
    }

    if (response_adu->GetSlaveId() != GetSlaveId()) {
        return userver::utils::unexpected{ClientError::kInvalidResponse};
    }

    const auto response_pdu = response_adu->GetPdu();
    if (response_pdu_out.size() < response_pdu.size()) {
        return userver::utils::unexpected{ClientError::kBufferTooSmall};
    }

    std::copy(response_pdu.begin(), response_pdu.end(), response_pdu_out.begin());

    return response_pdu.size();
}

void UdpClient::DrainSocket() {
    std::array<std::byte, RtuFrame::kMaxFrameSize> dummy_buf;
    while (socket_.RecvNoblock(dummy_buf.data(), dummy_buf.size()).has_value());
}

std::unique_ptr<Client> MakeUdpClient(
    std::uint8_t slave_id,
    ClientMetrics& metrics,
    userver::engine::io::Sockaddr endpoint,
    std::chrono::milliseconds timeout
) {
    return std::make_unique<UdpClient>(slave_id, metrics, endpoint, timeout);
}

}  // namespace modbus
