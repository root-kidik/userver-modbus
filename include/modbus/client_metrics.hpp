#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include <userver/utils/statistics/rate_counter.hpp>
#include <userver/utils/statistics/writer.hpp>

namespace modbus {

enum class MessageType : std::size_t {
    kReadCoils,
    kReadDiscreteInputs,
    kReadHoldingRegisters,
    kReadInputRegisters,
    kWriteSingleCoil,
    kWriteSingleRegister,
    kWriteMultipleCoils,
    kWriteMultipleRegisters,

    kMaxMessageType
};

constexpr std::string_view ToString(MessageType type) noexcept {
    switch (type) {
        case MessageType::kReadCoils:
            return "read_coils";
        case MessageType::kReadDiscreteInputs:
            return "read_discrete_inputs";
        case MessageType::kReadHoldingRegisters:
            return "read_holding_registers";
        case MessageType::kReadInputRegisters:
            return "read_input_registers";
        case MessageType::kWriteSingleCoil:
            return "write_single_coil";
        case MessageType::kWriteSingleRegister:
            return "write_single_register";
        case MessageType::kWriteMultipleCoils:
            return "write_multiple_coils";
        case MessageType::kWriteMultipleRegisters:
            return "write_multiple_registers";
        case MessageType::kMaxMessageType:
            break;
    }

    return "unknown";
}

struct MessageMetrics {
    userver::utils::statistics::RateCounter total;
    userver::utils::statistics::RateCounter success;
    userver::utils::statistics::RateCounter errors;
};

struct ClientMetrics {
    userver::utils::statistics::RateCounter bytes_sent;
    userver::utils::statistics::RateCounter bytes_received;

    userver::utils::statistics::RateCounter requests_total;
    userver::utils::statistics::RateCounter requests_success;
    userver::utils::statistics::RateCounter requests_errors;

    std::array<MessageMetrics, static_cast<std::size_t>(MessageType::kMaxMessageType)> by_message_type;
};

inline void DumpMetric(userver::utils::statistics::Writer& writer, const MessageMetrics& metrics) {
    writer["total"] = metrics.total;
    writer["success"] = metrics.success;
    writer["errors"] = metrics.errors;
}

inline void ResetMetric(MessageMetrics& metrics) {
    ResetMetric(metrics.total);
    ResetMetric(metrics.success);
    ResetMetric(metrics.errors);
}

inline void DumpMetric(userver::utils::statistics::Writer& writer, const ClientMetrics& metrics) {
    writer["bytes"]["sent"] = metrics.bytes_sent;
    writer["bytes"]["received"] = metrics.bytes_received;

    writer["requests"]["total"] = metrics.requests_total;
    writer["requests"]["success"] = metrics.requests_success;
    writer["requests"]["errors"] = metrics.requests_errors;

    auto by_type_writer = writer["by_type"];

    for (std::size_t i = 0; i < static_cast<std::size_t>(MessageType::kMaxMessageType); ++i) {
        by_type_writer[ToString(static_cast<MessageType>(i))] = metrics.by_message_type[i];
    }
}

inline void ResetMetric(ClientMetrics& metrics) {
    ResetMetric(metrics.bytes_sent);
    ResetMetric(metrics.bytes_received);
    ResetMetric(metrics.requests_total);
    ResetMetric(metrics.requests_success);
    ResetMetric(metrics.requests_errors);

    for (auto& item : metrics.by_message_type) {
        ResetMetric(item);
    }
}

}  // namespace modbus
