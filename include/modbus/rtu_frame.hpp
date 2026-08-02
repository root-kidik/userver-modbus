#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include <userver/utils/expected.hpp>

#include <modbus/constants.hpp>
#include <modbus/crc.hpp>

namespace modbus {

enum class RtuError : std::uint8_t { kBufferTooShort, kInvalidCrc, kPduTooLarge };

class RtuFrame {
public:
    static constexpr std::size_t kMinFrameSize = 3;
    static constexpr std::size_t kMaxFrameSize = 256;

    static userver::utils::expected<RtuFrame, RtuError> Create(std::uint8_t slave_id, std::span<const std::byte> pdu)
        noexcept {
        if (pdu.size() > kMaxPduSize) {
            return userver::utils::unexpected{RtuError::kPduTooLarge};
        }

        return RtuFrame{slave_id, pdu};
    }

    static userver::utils::expected<RtuFrame, RtuError> Deserialize(std::span<const std::byte>& buffer) {
        if (buffer.size() < kMinFrameSize) {
            return userver::utils::unexpected{RtuError::kBufferTooShort};
        }

        const auto pdu_size = buffer.size() - kMinFrameSize;

        if (pdu_size > kMaxPduSize) {
            return userver::utils::unexpected{RtuError::kPduTooLarge};
        }

        const auto slave_id = static_cast<std::uint8_t>(buffer[0]);

        const auto crc_lo = static_cast<std::uint8_t>(buffer[1 + pdu_size]);
        const auto crc_hi = static_cast<std::uint8_t>(buffer[1 + pdu_size + 1]);
        const auto received_crc = static_cast<std::uint16_t>((crc_hi << 8) | crc_lo);

        const auto calculated_crc = CalcCrc(buffer.subspan(0, buffer.size() - 2));

        if (calculated_crc != received_crc) {
            return userver::utils::unexpected{RtuError::kInvalidCrc};
        }

        const auto pdu_span = buffer.subspan(1, pdu_size);
        auto frame_res = Create(slave_id, pdu_span);
        if (!frame_res.has_value()) {
            return frame_res;
        }

        buffer = buffer.subspan(buffer.size());
        return frame_res;
    }

    userver::utils::expected<std::span<std::byte>, RtuError> Serialize(std::span<std::byte> out_buffer) const {
        const auto frame_size = GetFrameSize();
        if (out_buffer.size() < frame_size) {
            return userver::utils::unexpected{RtuError::kBufferTooShort};
        }

        out_buffer[0] = static_cast<std::byte>(slave_id_);
        std::copy_n(pdu_.begin(), pdu_size_, out_buffer.begin() + 1);

        CrcType crc;
        crc.process_byte(slave_id_);
        if (pdu_size_ > 0) {
            crc.process_bytes(pdu_.data(), pdu_size_);
        }

        const auto checksum = crc.checksum();

        out_buffer[1 + pdu_size_] = static_cast<std::byte>(checksum & 0xFF);
        out_buffer[1 + pdu_size_ + 1] = static_cast<std::byte>((checksum >> 8) & 0xFF);

        return out_buffer.subspan(frame_size);
    }

    [[nodiscard]] std::uint8_t GetSlaveId() const noexcept { return slave_id_; }

    [[nodiscard]] std::span<const std::byte> GetPdu() const noexcept {
        return std::span<const std::byte>{pdu_.data(), pdu_size_};
    }

    [[nodiscard]] std::size_t GetFrameSize() const noexcept { return pdu_size_ + kMinFrameSize; }

private:
    RtuFrame(std::uint8_t slave_id, std::span<const std::byte> pdu) noexcept
        : slave_id_{slave_id}, pdu_size_{static_cast<std::uint8_t>(pdu.size())} {
        std::copy(pdu.begin(), pdu.end(), pdu_.begin());
    }

    std::uint8_t slave_id_;
    std::uint8_t pdu_size_;

    std::array<std::byte, kMaxPduSize> pdu_{};
};

}  // namespace modbus
