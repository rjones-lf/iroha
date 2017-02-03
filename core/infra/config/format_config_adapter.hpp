
#ifndef ADAPTER_CONFIG_HPP
#define ADAPTER_CONFIG_HPP

#include <string>
#include "iroha_config.hpp"

namespace config { namespace formatChecking {
    // Destroy information hiding
    struct FormatConfigAdapter : public IConfig {
        json extractValue() {
            return *openConfig(getConfigName());
        }

        static FormatConfigAdapter &getInstance() {
            static FormatConfigAdapter instance;
            return instance;
        }

        std::string getConfigName() {
            return "core/infra/config/valid_format.json";
        }
    };
}}

#endif // ADAPTER_CONFIG_HPP