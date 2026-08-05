#include <modbus/client_component.hpp>

#include <userver/components/component_config.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include <modbus/udp_client.hpp>

namespace modbus {

ClientComponent::ClientComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : ComponentBase{config, context} {
    const auto transport = config["transport"].As<std::string>();
    const auto slave_id = config["slave_id"].As<std::uint8_t>();
    const auto timeout = config["timeout"].As<std::chrono::milliseconds>();
    const auto host = config["host"].As<std::string>();
    const auto port = config["port"].As<std::uint16_t>(502);

    auto& storage = context.FindComponent<userver::components::StatisticsStorage>().GetStorage();
    metrics_holder_ = storage.RegisterWriter(
        "modbus",
        [this](userver::utils::statistics::Writer& writer) { DumpMetric(writer, metrics_); },
        {{"client", config.Name()}}
    );

    if (transport == "udp") {
        auto endpoint = userver::engine::io::Sockaddr::MakeIPSocketAddress(host);
        endpoint.SetPort(port);

        client_ = MakeUdpClient(slave_id, metrics_, endpoint, timeout);
    }
}

userver::yaml_config::Schema ClientComponent::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<ComponentBase>(R"(
type: object
description: Modbus client component
additionalProperties: false
properties:
    transport:
        type: string
        enum:
          - udp
        description: Transport type
    slave_id:
        type: integer
        minimum: 1
        maximum: 247
        description: Modbus slave ID
    host:
        type: string
        description: Target IP
    port:
        type: integer
        default: 502
        minimum: 1
        maximum: 65535
        description: Target port
    timeout:
        type: string
        description: Request timeout (e.g. 500ms, 1s)
required:
  - transport
  - slave_id
  - host
  - timeout
)");
}

Client& ClientComponent::GetClient() noexcept { return *client_; }

}  // namespace modbus
