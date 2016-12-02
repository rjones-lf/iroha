//
// Created by SonokoMizuki on 2016/11/02.
//

#ifndef IROHA_JSON_PARSE_WITH_JSON_NLOHMAN_H
#define IROHA_JSON_PARSE_WITH_JSON_NLOHMAN_H

#include "../consensus/consensus_event.hpp"

// ***************8
// *
#include "../vendor/json/src/json.hpp"
// *
// ***************8


#include <type_traits>
#include <string>
#include <unordered_map>

namespace json_parse_with_json_nlohman {

    using json = nlohmann::json;

    using Object = json_parse::Object;
    using Rule = json_parse::Rule;
    using Type = json_parse::Type;

    namespace parser {

        namespace detail {
            json dump_impl(Object obj);
            Object load_impl(json& j,const Rule& r);
        }

        std::string dump(Object obj);

        template<typename T>
        std::unique_ptr<T> load(std::string s);

        Object load(json j);
    };

};
#endif //IROHA_JSON_PARSE_H
