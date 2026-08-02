#include <modbus/client_base.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <vector>

#include <userver/utest/utest.hpp>

namespace modbus::tests {

namespace {

class FakeModbusClient final : public ClientBase {
public:
    explicit FakeModbusClient(std::uint8_t slave_id = 1) : ClientBase(slave_id) {}

    void SetResponse(userver::utils::expected<std::vector<std::byte>, ClientError> response) {
        response_ = std::move(response);
    }

    const std::vector<std::byte>& GetLastSentPdu() const { return last_sent_pdu_; }

    userver::utils::expected<std::size_t, ClientError> SendRawRequest(
        std::span<const std::byte> request_pdu,
        std::span<std::byte> response_pdu_buf
    ) override {
        last_sent_pdu_.assign(request_pdu.begin(), request_pdu.end());

        if (!response_.has_value()) {
            return userver::utils::unexpected{response_.error()};
        }

        const auto& data = response_.value();
        if (response_pdu_buf.size() < data.size()) {
            return userver::utils::unexpected{ClientError::kBufferTooSmall};
        }

        std::copy(data.begin(), data.end(), response_pdu_buf.begin());
        return data.size();
    }

private:
    userver::utils::expected<std::vector<std::byte>, ClientError> response_{std::vector<std::byte>{}};
    std::vector<std::byte> last_sent_pdu_{};
};

}  // namespace

UTEST(ClientBaseTest, ReadCoilsSuccess) {
    FakeModbusClient client{1};

    const std::array<std::byte, 3> response_data{std::byte{0x01}, std::byte{0x01}, std::byte{0x05}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    std::array<Coil, 3> coils{};
    const auto res = client.ReadCoils(0x0010, 3, coils);

    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(coils[0], Coil::kOn);
    EXPECT_EQ(coils[1], Coil::kOff);
    EXPECT_EQ(coils[2], Coil::kOn);

    const std::array<std::byte, 5>
        expected_pdu{std::byte{0x01}, std::byte{0x00}, std::byte{0x10}, std::byte{0x00}, std::byte{0x03}};
    EXPECT_TRUE(std::ranges::equal(client.GetLastSentPdu(), expected_pdu));
}

UTEST(ClientBaseTest, ReadDiscreteInputsSuccess) {
    FakeModbusClient client{1};

    const std::array<std::byte, 3> response_data{std::byte{0x02}, std::byte{0x01}, std::byte{0x02}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    std::array<DiscreteInput, 2> inputs{};
    const auto res = client.ReadDiscreteInputs(0x0000, 2, inputs);

    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(inputs[0], DiscreteInput::kOff);
    EXPECT_EQ(inputs[1], DiscreteInput::kOn);
}

UTEST(ClientBaseTest, ReadHoldingRegistersSuccess) {
    FakeModbusClient client{1};

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
}

UTEST(ClientBaseTest, ReadInputRegistersSuccess) {
    FakeModbusClient client{1};

    const std::array<std::byte, 4> response_data{std::byte{0x04}, std::byte{0x02}, std::byte{0x00}, std::byte{0x2A}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    std::array<std::uint16_t, 1> registers{};
    const auto res = client.ReadInputRegisters(0x0001, 1, registers);

    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(registers[0], 42);
}

UTEST(ClientBaseTest, ReadBufferTooSmallError) {
    FakeModbusClient client{1};

    std::array<std::uint16_t, 1> small_buffer{};
    const auto res = client.ReadHoldingRegisters(0x0000, 2, small_buffer);

    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), ClientError::kBufferTooSmall);
}

UTEST(ClientBaseTest, WriteCoilSuccess) {
    FakeModbusClient client{1};

    const std::array<std::byte, 5>
        response_data{std::byte{0x05}, std::byte{0x00}, std::byte{0xAC}, std::byte{0xFF}, std::byte{0x00}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    const auto res = client.WriteCoil(0x00AC, Coil::kOn);

    ASSERT_TRUE(res.has_value());
    EXPECT_TRUE(std::ranges::equal(client.GetLastSentPdu(), response_data));
}

UTEST(ClientBaseTest, WriteHoldingRegisterSuccess) {
    FakeModbusClient client{1};

    const std::array<std::byte, 5>
        response_data{std::byte{0x06}, std::byte{0x00}, std::byte{0x01}, std::byte{0x03}, std::byte{0xE8}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    const auto res = client.WriteHoldingRegister(0x0001, 1000);

    ASSERT_TRUE(res.has_value());
    EXPECT_TRUE(std::ranges::equal(client.GetLastSentPdu(), response_data));
}

UTEST(ClientBaseTest, WriteCoilsSuccess) {
    FakeModbusClient client{1};

    const std::array<std::byte, 5>
        response_data{std::byte{0x0F}, std::byte{0x00}, std::byte{0x13}, std::byte{0x00}, std::byte{0x02}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    const std::array<Coil, 2> coils{Coil::kOn, Coil::kOff};
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
}

UTEST(ClientBaseTest, WriteHoldingRegistersSuccess) {
    FakeModbusClient client{1};

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
}

UTEST(ClientBaseTest, InvalidRequestQuantityZero) {
    FakeModbusClient client{1};

    std::array<std::uint16_t, 1> buf{};
    const auto res = client.ReadHoldingRegisters(0x0001, 0, buf);

    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), ClientError::kInvalidRequest);
}

UTEST(ClientBaseTest, TransportErrorPropagation) {
    FakeModbusClient client{1};
    client.SetResponse(userver::utils::unexpected{ClientError::kTimeout});

    std::array<std::uint16_t, 1> buf{};
    const auto res = client.ReadHoldingRegisters(0x0001, 1, buf);

    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), ClientError::kTimeout);
}

UTEST(ClientBaseTest, InvalidResponseDeserializationFailed) {
    FakeModbusClient client{1};

    const std::array<std::byte, 2> response_data{std::byte{0xFF}, std::byte{0x00}};
    client.SetResponse(std::vector<std::byte>{response_data.begin(), response_data.end()});

    std::array<std::uint16_t, 1> buf{};
    const auto res = client.ReadHoldingRegisters(0x0001, 1, buf);

    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), ClientError::kInvalidResponse);
}

UTEST(ClientBaseTest, GetSlaveIdReturnsCorrectValue) {
    FakeModbusClient client{42};
    EXPECT_EQ(client.GetSlaveId(), 42);
}

}  // namespace modbus::tests
