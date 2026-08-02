#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include <modbus/coil.hpp>
#include <modbus/discrete_input.hpp>

namespace modbus {

class Client {
public:
    virtual ~Client() = default;

    virtual std::uint8_t GetSlaveId() const noexcept = 0;

    virtual Coil ReadCoil(std::uint16_t address) = 0;
    virtual std::vector<Coil> ReadCoils(std::uint16_t address, std::uint16_t quantity) = 0;

    virtual DiscreteInput ReadDiscreteInput(std::uint16_t address) = 0;
    virtual std::vector<DiscreteInput> ReadDiscreteInputs(std::uint16_t address, std::uint16_t quantity) = 0;

    virtual std::uint16_t ReadHoldingRegister(std::uint16_t address) = 0;
    virtual std::vector<std::uint16_t> ReadHoldingRegisters(std::uint16_t address, std::uint16_t quantity) = 0;

    virtual std::uint16_t ReadInputRegister(std::uint16_t address) = 0;
    virtual std::vector<std::uint16_t> ReadInputRegisters(std::uint16_t address, std::uint16_t quantity) = 0;

    virtual void WriteCoil(std::uint16_t address, Coil value) = 0;
    virtual void WriteCoils(std::uint16_t address, std::span<Coil> values) = 0;

    virtual void WriteHoldingRegister(std::uint16_t address, std::uint16_t value) = 0;
    virtual void WriteHoldingRegisters(std::uint16_t address, std::span<std::uint16_t> values) = 0;

    virtual std::vector<std::byte> SendRawRequest(std::span<std::byte> pdu) = 0;
};

}  // namespace modbus
