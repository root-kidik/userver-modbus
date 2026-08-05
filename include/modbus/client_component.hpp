#pragma once

#include <memory>

#include <userver/components/component_base.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/statistics_storage.hpp>

#include <modbus/client.hpp>
#include <modbus/client_metrics.hpp>

namespace modbus {

class ClientComponent final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "modbus-client";

    ClientComponent(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

    static userver::yaml_config::Schema GetStaticConfigSchema();

    Client& GetClient() noexcept;

private:
    ClientMetrics metrics_;
    userver::utils::statistics::Entry metrics_holder_;

    std::unique_ptr<Client> client_;
};

}  // namespace modbus
