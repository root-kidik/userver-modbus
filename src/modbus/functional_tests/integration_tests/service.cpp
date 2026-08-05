#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/utils/encoding/hex.hpp>

#include <modbus/client_component.hpp>
#include <modbus/constants.hpp>

class ModbusTestHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-modbus-test";

    ModbusTestHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    )
        : HttpHandlerBase{config, context},
          modbus_client_{context.FindComponent<modbus::ClientComponent>("modbus-client-test").GetClient()} {}

    std::string
    HandleRequestThrow(const userver::server::http::HttpRequest& request, userver::server::request::RequestContext&)
        const override {
        const auto request_json = userver::formats::json::FromString(request.RequestBody());
        const auto action = request_json["action"].As<std::string>();

        userver::formats::json::ValueBuilder result_json;

        if (action == "get_slave_id") {
            result_json["slave_id"] = modbus_client_.GetSlaveId();
            return userver::formats::json::ToString(result_json.ExtractValue());
        }

        auto handle_error = [&](const auto&) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
            userver::formats::json::ValueBuilder err_json;
            err_json["status"] = "error";
            return userver::formats::json::ToString(err_json.ExtractValue());
        };

        if (action == "read_coils") {
            const auto address = request_json["address"].As<std::uint16_t>();
            const auto quantity = request_json["quantity"].As<std::uint16_t>();

            std::vector<modbus::Coil> coils(quantity);

            if (const auto result = modbus_client_.ReadCoils(address, quantity, coils); !result) {
                return handle_error(result.error());
            }

            userver::formats::json::ValueBuilder coils_array{userver::formats::json::Type::kArray};

            for (const auto& coil : coils) {
                coils_array.PushBack(static_cast<bool>(coil));
            }

            result_json["values"] = std::move(coils_array);
        } else if (action == "read_discrete_inputs") {
            const auto address = request_json["address"].As<std::uint16_t>();
            const auto quantity = request_json["quantity"].As<std::uint16_t>();

            std::vector<modbus::DiscreteInput> inputs(quantity);

            if (const auto result = modbus_client_.ReadDiscreteInputs(address, quantity, inputs); !result) {
                return handle_error(result.error());
            }

            userver::formats::json::ValueBuilder inputs_arr(userver::formats::json::Type::kArray);

            for (const auto& input : inputs) {
                inputs_arr.PushBack(static_cast<bool>(input));
            }

            result_json["values"] = std::move(inputs_arr);
        } else if (action == "read_holding_registers") {
            const auto address = request_json["address"].As<std::uint16_t>();
            const auto quantity = request_json["quantity"].As<std::uint16_t>();

            std::vector<std::uint16_t> registers(quantity);

            if (const auto result = modbus_client_.ReadHoldingRegisters(address, quantity, registers); !result) {
                return handle_error(result.error());
            }

            result_json["values"] = registers;
        } else if (action == "read_input_registers") {
            const auto address = request_json["address"].As<std::uint16_t>();
            const auto quantity = request_json["quantity"].As<std::uint16_t>();

            std::vector<std::uint16_t> registers(quantity);

            if (const auto result = modbus_client_.ReadInputRegisters(address, quantity, registers); !result) {
                return handle_error(result.error());
            }

            result_json["values"] = registers;
        } else if (action == "write_coil") {
            const auto address = request_json["address"].As<std::uint16_t>();
            const auto value = request_json["value"].As<bool>();

            if (const auto result = modbus_client_.WriteCoil(address, modbus::Coil{value}); !result) {
                return handle_error(result.error());
            }
        } else if (action == "write_coils") {
            const auto address = request_json["address"].As<std::uint16_t>();
            const auto bool_values = request_json["values"].As<std::vector<bool>>();

            std::vector<modbus::Coil> coils;
            coils.reserve(bool_values.size());
            for (auto b : bool_values) {
                coils.push_back(modbus::Coil{b});
            }

            if (const auto result = modbus_client_.WriteCoils(address, coils); !result) {
                return handle_error(result.error());
            }
        } else if (action == "write_holding_register") {
            const auto address = request_json["address"].As<std::uint16_t>();
            const auto value = request_json["value"].As<std::uint16_t>();

            if (const auto result = modbus_client_.WriteHoldingRegister(address, value); !result) {
                return handle_error(result.error());
            }
        } else if (action == "write_holding_registers") {
            const auto address = request_json["address"].As<std::uint16_t>();
            const auto vals = request_json["values"].As<std::vector<std::uint16_t>>();

            if (const auto result = modbus_client_.WriteHoldingRegisters(address, vals); !result) {
                return handle_error(result.error());
            }
        } else if (action == "send_raw_request") {
            const auto hex_pdu = request_json["request_pdu"].As<std::string>();
            const auto raw_request_string = userver::utils::encoding::FromHex(hex_pdu);

            std::vector<std::byte> request_pdu(raw_request_string.size());
            std::memcpy(request_pdu.data(), raw_request_string.data(), raw_request_string.size());

            std::vector<std::byte> response_pdu(modbus::kMaxPduSize);
            const auto result = modbus_client_.SendRawRequest(request_pdu, response_pdu);
            if (!result) {
                return handle_error(result.error());
            }

            std::string_view response_view{reinterpret_cast<const char*>(response_pdu.data()), result.value()};

            result_json["response_pdu"] = userver::utils::encoding::ToHex(response_view);
        } else {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            result_json["error"] = "Unknown action: " + action;
            return userver::formats::json::ToString(result_json.ExtractValue());
        }

        result_json["status"] = "ok";
        return userver::formats::json::ToString(result_json.ExtractValue());
    }

private:
    modbus::Client& modbus_client_;
};

int main(int argc, char* argv[]) {
    const auto component_list =
        userver::components::MinimalServerComponentList()
            .AppendComponentList(userver::clients::http::ComponentList())
            .Append<userver::components::TestsuiteSupport>()
            .Append<userver::server::handlers::TestsControl>()
            .Append<userver::clients::dns::Component>()
            .Append<ModbusTestHandler>()
            .Append<modbus::ClientComponent>("modbus-client-test");

    return userver::utils::DaemonMain(argc, argv, component_list);
}
