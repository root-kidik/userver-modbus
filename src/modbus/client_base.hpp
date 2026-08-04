#pragma once

#include <cstdint>
#include <span>

#include <modbus/client.hpp>
#include <modbus/client_metrics.hpp>

namespace modbus {

class ClientBase : public Client {
public:
    ClientBase(std::uint8_t slave_id, ClientMetrics& metrics);

    [[nodiscard]] std::uint8_t GetSlaveId() const noexcept override;

    userver::utils::expected<void, ClientError> ReadCoils(
        std::uint16_t address,
        std::uint16_t quantity,
        std::span<Coil> out_coils
    ) override;

    userver::utils::expected<void, ClientError> ReadDiscreteInputs(
        std::uint16_t address,
        std::uint16_t quantity,
        std::span<DiscreteInput> out_inputs
    ) override;

    userver::utils::expected<void, ClientError> ReadHoldingRegisters(
        std::uint16_t address,
        std::uint16_t quantity,
        std::span<std::uint16_t> out_registers
    ) override;

    userver::utils::expected<void, ClientError> ReadInputRegisters(
        std::uint16_t address,
        std::uint16_t quantity,
        std::span<std::uint16_t> out_registers
    ) override;

    userver::utils::expected<void, ClientError> WriteCoil(std::uint16_t address, Coil value) override;

    userver::utils::expected<void, ClientError> WriteCoils(std::uint16_t address, std::span<const Coil> values)
        override;

    userver::utils::expected<void, ClientError> WriteHoldingRegister(std::uint16_t address, std::uint16_t value)
        override;

    userver::utils::expected<void, ClientError> WriteHoldingRegisters(
        std::uint16_t address,
        std::span<const std::uint16_t> values
    ) override;

private:
    template <typename Request>
    userver::utils::expected<std::size_t, ClientError> SendPdu(
        const Request& request,
        std::span<std::byte> response_pdu_buf
    );

    template <typename Request, typename Response, typename T>
    userver::utils::expected<void, ClientError> ExecuteRead(
        std::uint16_t address,
        std::uint16_t quantity,
        std::span<T> out_buffer
    );

    template <typename Request, typename Response, typename ValueT>
    userver::utils::expected<void, ClientError> ExecuteWriteSingle(std::uint16_t address, ValueT value);

    template <typename Request, typename Response, typename ElementT>
    userver::utils::expected<void, ClientError> ExecuteWriteMultiple(
        std::uint16_t address,
        std::span<const ElementT> values
    );

    std::uint8_t slave_id_;

    ClientMetrics& metrics_;
};

}  // namespace modbus
