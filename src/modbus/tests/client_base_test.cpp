#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <vector>

#include <userver/utest/utest.hpp>

#include <modbus/client_base.hpp>
#include <modbus/client_metrics.hpp>

namespace {

class FakeModbusClient final : public modbus::ClientBase {
public:
    explicit FakeModbusClient(modbus::ClientMetrics& metrics, std::uint8_t slave_id = 1)
        : modbus::ClientBase(slave_id, metrics) {}

    void SetResponse(userver::utils::expected<std::vector<std::byte>, modbus::ClientError> response) {
        response_ = std::move(response);
    }

    const std::vector<std::byte>& GetLastSentPdu() const { return last_sent_pdu_; }

    userver::utils::expected<std::size_t, modbus::ClientError> SendRawRequest(
        std::span<const std::byte> request_pdu,
        std::span<std::byte> response_pdu_buf
    ) override {
        last_sent_pdu_.assign(request_pdu.begin(), request_pdu.end());

        if (!response_.has_value()) {
            return userver::utils::unexpected{response_.error()};
        }

        const auto& data = response_.value();
        if (response_pdu_buf.size() < data.size()) {
            return userver::utils::unexpected{modbus::ClientError::kBufferTooSmall};
        }

        std::copy(data.begin(), data.end(), response_pdu_buf.begin());
        return data.size();
    }

private:
    userver::utils::expected<std::vector<std::byte>, modbus::ClientError> response_{std::vector<std::byte>{}};
    std::vector<std::byte> last_sent_pdu_{};
};

struct ClientBaseTest : testing::Test {
    modbus::ClientMetrics metrics{};
    FakeModbusClient client{metrics, 1};

    const modbus::MessageMetrics& GetMsgMetrics(modbus::MessageType type) const {
        return metrics.by_message_type[static_cast<std::size_t>(type)];
    }
};

}  // namespace

UTEST_F(ClientBaseTest, ReadCoilsSuccess) {
    const std::array<std::byte, 3> response_data{std::byte{0x01}, std::byte{0x01}, std::byte{0x05}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    std::array<modbus::Coil, 3> coils{};
    const auto res = client.ReadCoils(0x0010, 3, coils);

    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(coils[0], modbus::Coil::kOn);
    EXPECT_EQ(coils[1], modbus::Coil::kOff);
    EXPECT_EQ(coils[2], modbus::Coil::kOn);

    const std::array<std::byte, 5>
        expected_pdu{std::byte{0x01}, std::byte{0x00}, std::byte{0x10}, std::byte{0x00}, std::byte{0x03}};
    EXPECT_TRUE(std::ranges::equal(client.GetLastSentPdu(), expected_pdu));

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_success.Load().value, 1);
    EXPECT_EQ(metrics.requests_errors.Load().value, 0);

    EXPECT_EQ(metrics.bytes_sent.Load().value, expected_pdu.size());
    EXPECT_EQ(metrics.bytes_received.Load().value, response_data.size());

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kReadCoils);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.success.Load().value, 1);
    EXPECT_EQ(msg_m.errors.Load().value, 0);
}

UTEST_F(ClientBaseTest, ReadDiscreteInputsSuccess) {
    const std::array<std::byte, 3> response_data{std::byte{0x02}, std::byte{0x01}, std::byte{0x02}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    std::array<modbus::DiscreteInput, 2> inputs{};
    const auto res = client.ReadDiscreteInputs(0x0000, 2, inputs);

    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(inputs[0], modbus::DiscreteInput::kOff);
    EXPECT_EQ(inputs[1], modbus::DiscreteInput::kOn);

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_success.Load().value, 1);

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kReadDiscreteInputs);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.success.Load().value, 1);
}

UTEST_F(ClientBaseTest, ReadHoldingRegistersSuccess) {
    const std::array<std::byte, 6> response_data{
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x12},
        std::byte{0x34},
        std::byte{0x56},
        std::byte{0x78}
    };
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    std::array<std::uint16_t, 2> registers{};
    const auto res = client.ReadHoldingRegisters(0x006B, 2, registers);

    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(registers[0], 0x1234);
    EXPECT_EQ(registers[1], 0x5678);

    const std::array<std::byte, 5>
        expected_pdu{std::byte{0x03}, std::byte{0x00}, std::byte{0x6B}, std::byte{0x00}, std::byte{0x02}};
    EXPECT_TRUE(std::ranges::equal(client.GetLastSentPdu(), expected_pdu));

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_success.Load().value, 1);

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kReadHoldingRegisters);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.success.Load().value, 1);
}

UTEST_F(ClientBaseTest, ReadInputRegistersSuccess) {
    const std::array<std::byte, 4> response_data{std::byte{0x04}, std::byte{0x02}, std::byte{0x00}, std::byte{0x2A}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    std::array<std::uint16_t, 1> registers{};
    const auto res = client.ReadInputRegisters(0x0001, 1, registers);

    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(registers[0], 42);

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_success.Load().value, 1);

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kReadInputRegisters);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.success.Load().value, 1);
}

UTEST_F(ClientBaseTest, ReadBufferTooSmallError) {
    std::array<std::uint16_t, 1> small_buffer{};
    const auto res = client.ReadHoldingRegisters(0x0000, 2, small_buffer);

    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), modbus::ClientError::kBufferTooSmall);

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_success.Load().value, 0);
    EXPECT_EQ(metrics.requests_errors.Load().value, 1);

    EXPECT_EQ(metrics.bytes_sent.Load().value, 0);
    EXPECT_EQ(metrics.bytes_received.Load().value, 0);

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kReadHoldingRegisters);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.errors.Load().value, 1);
}

UTEST_F(ClientBaseTest, WriteCoilSuccess) {
    const std::array<std::byte, 5>
        response_data{std::byte{0x05}, std::byte{0x00}, std::byte{0xAC}, std::byte{0xFF}, std::byte{0x00}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    const auto res = client.WriteCoil(0x00AC, modbus::Coil::kOn);

    ASSERT_TRUE(res.has_value());
    EXPECT_TRUE(std::ranges::equal(client.GetLastSentPdu(), response_data));

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_success.Load().value, 1);

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kWriteSingleCoil);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.success.Load().value, 1);
}

UTEST_F(ClientBaseTest, WriteHoldingRegisterSuccess) {
    const std::array<std::byte, 5>
        response_data{std::byte{0x06}, std::byte{0x00}, std::byte{0x01}, std::byte{0x03}, std::byte{0xE8}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    const auto res = client.WriteHoldingRegister(0x0001, 1000);

    ASSERT_TRUE(res.has_value());
    EXPECT_TRUE(std::ranges::equal(client.GetLastSentPdu(), response_data));

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_success.Load().value, 1);

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kWriteSingleRegister);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.success.Load().value, 1);
}

UTEST_F(ClientBaseTest, WriteCoilsSuccess) {
    const std::array<std::byte, 5>
        response_data{std::byte{0x0F}, std::byte{0x00}, std::byte{0x13}, std::byte{0x00}, std::byte{0x02}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    const std::array<modbus::Coil, 2> coils{modbus::Coil::kOn, modbus::Coil::kOff};
    const auto res = client.WriteCoils(0x0013, coils);

    ASSERT_TRUE(res.has_value());

    const std::array<std::byte, 7> expected_pdu{
        std::byte{0x0F},
        std::byte{0x00},
        std::byte{0x13},
        std::byte{0x00},
        std::byte{0x02},
        std::byte{0x01},
        std::byte{0x01}
    };
    EXPECT_TRUE(std::ranges::equal(client.GetLastSentPdu(), expected_pdu));

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_success.Load().value, 1);

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kWriteMultipleCoils);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.success.Load().value, 1);
}

UTEST_F(ClientBaseTest, WriteHoldingRegistersSuccess) {
    const std::array<std::byte, 5>
        response_data{std::byte{0x10}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x02}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    const std::array<std::uint16_t, 2> registers{0x000A, 0x0102};
    const auto res = client.WriteHoldingRegisters(0x0001, registers);

    ASSERT_TRUE(res.has_value());

    const std::array<std::byte, 10> expected_pdu{
        std::byte{0x10},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x00},
        std::byte{0x02},
        std::byte{0x04},
        std::byte{0x00},
        std::byte{0x0A},
        std::byte{0x01},
        std::byte{0x02}
    };
    EXPECT_TRUE(std::ranges::equal(client.GetLastSentPdu(), expected_pdu));

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_success.Load().value, 1);

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kWriteMultipleRegisters);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.success.Load().value, 1);
}

UTEST_F(ClientBaseTest, InvalidRequestQuantityZero) {
    std::array<std::uint16_t, 1> buf{};
    const auto res = client.ReadHoldingRegisters(0x0001, 0, buf);

    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), modbus::ClientError::kInvalidRequest);

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_errors.Load().value, 1);
    EXPECT_EQ(metrics.bytes_sent.Load().value, 0);

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kReadHoldingRegisters);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.errors.Load().value, 1);
}

UTEST_F(ClientBaseTest, TransportErrorPropagation) {
    client.SetResponse(userver::utils::unexpected{modbus::ClientError::kTimeout});

    std::array<std::uint16_t, 1> buf{};
    const auto res = client.ReadHoldingRegisters(0x0001, 1, buf);

    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), modbus::ClientError::kTimeout);

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_errors.Load().value, 1);
    EXPECT_EQ(metrics.bytes_sent.Load().value, 5);
    EXPECT_EQ(metrics.bytes_received.Load().value, 0);

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kReadHoldingRegisters);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.errors.Load().value, 1);
}

UTEST_F(ClientBaseTest, InvalidResponseDeserializationFailed) {
    const std::array<std::byte, 2> response_data{std::byte{0xFF}, std::byte{0x00}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    std::array<std::uint16_t, 1> buf{};
    const auto res = client.ReadHoldingRegisters(0x0001, 1, buf);

    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), modbus::ClientError::kInvalidResponse);

    EXPECT_EQ(metrics.requests_total.Load().value, 1);
    EXPECT_EQ(metrics.requests_errors.Load().value, 1);
    EXPECT_EQ(metrics.bytes_sent.Load().value, 5);
    EXPECT_EQ(metrics.bytes_received.Load().value, response_data.size());

    const auto& msg_m = GetMsgMetrics(modbus::MessageType::kReadHoldingRegisters);
    EXPECT_EQ(msg_m.total.Load().value, 1);
    EXPECT_EQ(msg_m.errors.Load().value, 1);
}

UTEST_F(ClientBaseTest, GetSlaveIdReturnsCorrectValue) { EXPECT_EQ(client.GetSlaveId(), 1); }
