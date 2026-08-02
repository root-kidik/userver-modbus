#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
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

    [[nodiscard]] static userver::utils::expected<RtuFrame, RtuError> Create(
        std::uint8_t slave_id,
        std::span<const std::byte> pdu
    ) noexcept {
        if (pdu.size() > kMaxPduSize) {
            return userver::utils::unexpected{RtuError::kPduTooLarge};
        }

        return RtuFrame{slave_id, pdu};
    }

    template <typename InputIt>
    [[nodiscard]] static userver::utils::expected<RtuFrame, RtuError> Deserialize(InputIt& first, InputIt last) {
        const auto distance = std::distance(first, last);

        if (distance < static_cast<decltype(distance)>(kMinFrameSize)) {
            return userver::utils::unexpected{RtuError::kBufferTooShort};
        }

        const auto pdu_size = distance - kMinFrameSize;

        if (pdu_size > kMaxPduSize) {
            return userver::utils::unexpected{RtuError::kPduTooLarge};
        }

        auto crc_it = first;
        std::advance(crc_it, distance - 2);

        const auto calculated_crc = CalcCrc(first, crc_it);

        const auto slave_id = static_cast<std::uint8_t>(*first++);

        std::array<std::byte, kMaxPduSize> pdu_buf{};
        for (std::size_t i = 0; i < static_cast<std::size_t>(pdu_size); ++i) {
            pdu_buf[i] = static_cast<std::byte>(*first++);
        }

        const auto crc_lo = static_cast<std::uint8_t>(*first++);
        const auto crc_hi = static_cast<std::uint8_t>(*first++);

        const auto received_crc = static_cast<std::uint16_t>((crc_hi << 8) | crc_lo);

        if (calculated_crc != received_crc) {
            return userver::utils::unexpected{RtuError::kInvalidCrc};
        }

        return Create(slave_id, std::span<const std::byte>{pdu_buf.data(), static_cast<std::size_t>(pdu_size)});
    }

    template <typename OutputIt>
    [[nodiscard]] OutputIt Serialize(OutputIt out) const {
        *out++ = static_cast<std::byte>(slave_id_);

        out = std::copy_n(pdu_.begin(), pdu_size_, out);

        CrcType crc;
        crc.process_byte(slave_id_);

        if (pdu_size_ > 0) {
            crc.process_bytes(pdu_.data(), pdu_size_);
        }

        const auto checksum = crc.checksum();

        *out++ = static_cast<std::byte>(checksum & 0xFF);
        *out++ = static_cast<std::byte>((checksum >> 8) & 0xFF);

        return out;
    }

    [[nodiscard]] std::uint8_t GetSlaveId() const noexcept { return slave_id_; }

    [[nodiscard]] std::span<const std::byte> GetPdu() const noexcept {
        return std::span<const std::byte>{pdu_.data(), pdu_size_};
    }

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
