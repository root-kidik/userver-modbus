#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <span>

#include <userver/engine/io/sockaddr.hpp>
#include <userver/utils/expected.hpp>

#include <modbus/client_metrics.hpp>
#include <modbus/coil.hpp>
#include <modbus/discrete_input.hpp>

namespace modbus {

enum class ClientError : std::uint8_t {
    kBufferTooSmall,
    kInvalidRequest,
    kInvalidResponse,
    kTransportError,
    kTimeout,
    kModbusException
};

class Client {
public:
    virtual ~Client() = default;

    [[nodiscard]] virtual std::uint8_t GetSlaveId() const noexcept = 0;

    virtual userver::utils::expected<void, ClientError> ReadCoils(
        std::uint16_t address,
        std::uint16_t quantity,
        std::span<Coil> out_coils
    ) = 0;

    virtual userver::utils::expected<void, ClientError> ReadDiscreteInputs(
        std::uint16_t address,
        std::uint16_t quantity,
        std::span<DiscreteInput> out_inputs
    ) = 0;

    virtual userver::utils::expected<void, ClientError> ReadHoldingRegisters(
        std::uint16_t address,
        std::uint16_t quantity,
        std::span<std::uint16_t> out_registers
    ) = 0;

    virtual userver::utils::expected<void, ClientError> ReadInputRegisters(
        std::uint16_t address,
        std::uint16_t quantity,
        std::span<std::uint16_t> out_registers
    ) = 0;

    virtual userver::utils::expected<void, ClientError> WriteCoil(std::uint16_t address, Coil value) = 0;

    virtual userver::utils::expected<void, ClientError> WriteCoils(
        std::uint16_t address,
        std::span<const Coil> values
    ) = 0;

    virtual userver::utils::expected<void, ClientError> WriteHoldingRegister(
        std::uint16_t address,
        std::uint16_t value
    ) = 0;

    virtual userver::utils::expected<void, ClientError> WriteHoldingRegisters(
        std::uint16_t address,
        std::span<const std::uint16_t> values
    ) = 0;

    virtual userver::utils::expected<std::size_t, ClientError> SendRawRequest(
        std::span<const std::byte> request_pdu,
        std::span<std::byte> response_pdu_out
    ) = 0;
};

std::unique_ptr<Client> MakeUdpClient(
    std::uint8_t slave_id,
    ClientMetrics& metrics,
    userver::engine::io::Sockaddr endpoint,
    std::chrono::milliseconds timeout
);

}  // namespace modbus
