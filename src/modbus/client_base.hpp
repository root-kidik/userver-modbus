#pragma once

#include <modbus/client.hpp>

namespace modbus {

class ClientBase : public Client {
public:
    ClientBase(std::uint8_t slave_id);

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
    std::uint8_t slave_id_;
};

}  // namespace modbus
