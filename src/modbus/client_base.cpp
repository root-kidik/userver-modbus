#include <modbus/client_base.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>

#include <modbus/request/read_coils.hpp>
#include <modbus/request/read_discrete_inputs.hpp>
#include <modbus/request/read_holding_registers.hpp>
#include <modbus/request/read_input_registers.hpp>
#include <modbus/request/write_multiple_coils.hpp>
#include <modbus/request/write_multiple_holding_registers.hpp>
#include <modbus/request/write_single_coil.hpp>
#include <modbus/request/write_single_holding_register.hpp>

#include <modbus/response/read_coils.hpp>
#include <modbus/response/read_discrete_inputs.hpp>
#include <modbus/response/read_holding_registers.hpp>
#include <modbus/response/read_input_registers.hpp>
#include <modbus/response/write_multiple_coils.hpp>
#include <modbus/response/write_multiple_holding_registers.hpp>
#include <modbus/response/write_single_coil.hpp>
#include <modbus/response/write_single_holding_register.hpp>

namespace modbus {

namespace {

constexpr std::size_t kMaxPduSize = 253;

template <typename Request>
userver::utils::expected<std::size_t, ClientError> SendPdu(
    ClientBase& client,
    const Request& request,
    std::span<std::byte> response_pdu_buf
) {
    std::array<std::byte, kMaxPduSize> request_pdu_raw{};
    std::span<std::byte> request_buf{request_pdu_raw};

    const auto ser_result = request.Serialize(request_buf);
    if (!ser_result.has_value()) {
        return userver::utils::unexpected{ClientError::kInvalidRequest};
    }

    const std::size_t request_pdu_len = request_pdu_raw.size() - ser_result->size();
    const std::span<const std::byte> request_pdu{request_pdu_raw.data(), request_pdu_len};

    const auto send_res = client.SendRawRequest(request_pdu, response_pdu_buf);
    if (!send_res.has_value()) {
        return userver::utils::unexpected{send_res.error()};
    }

    return *send_res;
}

template <typename Request, typename Response, typename T>
userver::utils::expected<void, ClientError> ExecuteRead(
    ClientBase& client,
    std::uint16_t address,
    std::uint16_t quantity,
    std::span<T> out_buffer
) {
    if (out_buffer.size() < quantity) {
        return userver::utils::unexpected{ClientError::kBufferTooSmall};
    }

    const auto request = Request::Create(address, quantity);
    if (!request.has_value()) {
        return userver::utils::unexpected{ClientError::kInvalidRequest};
    }

    std::array<std::byte, kMaxPduSize> response_pdu_raw{};
    const auto pdu_len = SendPdu(client, *request, response_pdu_raw);
    if (!pdu_len.has_value()) {
        return userver::utils::unexpected{pdu_len.error()};
    }

    std::span<const std::byte> response_pdu{response_pdu_raw.data(), *pdu_len};

    const auto response = Response::Deserialize(response_pdu, quantity);
    if (!response.has_value()) {
        return userver::utils::unexpected{ClientError::kInvalidResponse};
    }

    const auto values = response->GetValues();
    std::copy(values.begin(), values.end(), out_buffer.begin());

    return {};
}

template <typename Request, typename Response, typename ValueT>
userver::utils::expected<void, ClientError> ExecuteWriteSingle(
    ClientBase& client,
    std::uint16_t address,
    ValueT value
) {
    const auto request = Request::Create(address, value);
    if (!request.has_value()) {
        return userver::utils::unexpected{ClientError::kInvalidRequest};
    }

    std::array<std::byte, kMaxPduSize> response_pdu_raw{};
    const auto pdu_len = SendPdu(client, *request, response_pdu_raw);
    if (!pdu_len.has_value()) {
        return userver::utils::unexpected{pdu_len.error()};
    }

    std::span<const std::byte> response_pdu{response_pdu_raw.data(), *pdu_len};

    const auto response = Response::Deserialize(response_pdu);
    if (!response.has_value()) {
        return userver::utils::unexpected{ClientError::kInvalidResponse};
    }

    return {};
}

template <typename Request, typename Response, typename ElementT>
userver::utils::expected<void, ClientError> ExecuteWriteMultiple(
    ClientBase& client,
    std::uint16_t address,
    std::span<const ElementT> values
) {
    const auto request = Request::Create(address, values);
    if (!request.has_value()) {
        return userver::utils::unexpected{ClientError::kInvalidRequest};
    }

    std::array<std::byte, kMaxPduSize> response_pdu_raw{};
    const auto pdu_len = SendPdu(client, *request, response_pdu_raw);
    if (!pdu_len.has_value()) {
        return userver::utils::unexpected{pdu_len.error()};
    }

    std::span<const std::byte> response_pdu{response_pdu_raw.data(), *pdu_len};

    const auto response = Response::Deserialize(response_pdu);
    if (!response.has_value()) {
        return userver::utils::unexpected{ClientError::kInvalidResponse};
    }

    return {};
}

}  // namespace

ClientBase::ClientBase(std::uint8_t slave_id) : slave_id_{slave_id} {}

std::uint8_t ClientBase::GetSlaveId() const noexcept { return slave_id_; }

userver::utils::expected<void, ClientError> ClientBase::ReadCoils(
    std::uint16_t address,
    std::uint16_t quantity,
    std::span<Coil> out_coils
) {
    return ExecuteRead<request::ReadCoils, response::ReadCoils>(*this, address, quantity, out_coils);
}

userver::utils::expected<void, ClientError> ClientBase::ReadDiscreteInputs(
    std::uint16_t address,
    std::uint16_t quantity,
    std::span<DiscreteInput> out_inputs
) {
    return ExecuteRead<request::ReadDiscreteInputs, response::ReadDiscreteInputs>(*this, address, quantity, out_inputs);
}

userver::utils::expected<void, ClientError> ClientBase::ReadHoldingRegisters(
    std::uint16_t address,
    std::uint16_t quantity,
    std::span<std::uint16_t> out_registers
) {
    return ExecuteRead<
        request::ReadHoldingRegisters,
        response::ReadHoldingRegisters>(*this, address, quantity, out_registers);
}

userver::utils::expected<void, ClientError> ClientBase::ReadInputRegisters(
    std::uint16_t address,
    std::uint16_t quantity,
    std::span<std::uint16_t> out_registers
) {
    return ExecuteRead<
        request::ReadInputRegisters,
        response::ReadInputRegisters>(*this, address, quantity, out_registers);
}

userver::utils::expected<void, ClientError> ClientBase::WriteCoil(std::uint16_t address, Coil value) {
    return ExecuteWriteSingle<request::WriteSingleCoil, response::WriteSingleCoil>(*this, address, value);
}

userver::utils::expected<void, ClientError> ClientBase::WriteCoils(
    std::uint16_t address,
    std::span<const Coil> values
) {
    return ExecuteWriteMultiple<request::WriteMultipleCoils, response::WriteMultipleCoils>(*this, address, values);
}

userver::utils::expected<void, ClientError> ClientBase::WriteHoldingRegister(
    std::uint16_t address,
    std::uint16_t value
) {
    return ExecuteWriteSingle<
        request::WriteSingleHoldingRegister,
        response::WriteSingleHoldingRegister>(*this, address, value);
}

userver::utils::expected<void, ClientError> ClientBase::WriteHoldingRegisters(
    std::uint16_t address,
    std::span<const std::uint16_t> values
) {
    return ExecuteWriteMultiple<
        request::WriteMultipleHoldingRegisters,
        response::WriteMultipleHoldingRegisters>(*this, address, values);
}

}  // namespace modbus
