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

#include "json_parse.hpp"

namespace json_parse_with_json_nlohman {

    using json = nlohmann::json;
    using Object = json_parse::Object;
    using Rule = json_parse::Rule;
    using Type = json_parse::Type;

    template<
        typename T//,
        //class = typename std::enable_if<validator_json<T>::value>::type
    >
    class JsonParse {

       json dump_impl(Object obj){
            json res;
            if(obj.getType() == Type::DICT){
                for(auto&& o : obj.dictSub){
                    res[o.first] = dump_impl(o.second);
                }
            }else if(obj.getType() == Type::LIST){
                res = json::array();
                for(auto&& o : obj.listSub){
                    res.push_back(dump_impl(o));
                }
            }else {
                if (obj.getType() == Type::INT) {
                    res = std::to_string(obj.integer);
                } else if (obj.getType() == Type::BOOL) {
                    res = obj.boolean ? "true" : "false";
                } else if (obj.getType() == Type::STR) {
                    res = obj.str;
                } else if (obj.getType() == Type::FLOAT) {
                    res = std::to_string(obj.floating);
                }
            }
            return res;
        }

       Object load_impl(json j,Rule r){
            if(r.getType() == Type::DICT) {
                std::map<std::string, Rule> dict = r.dictSub;
                Object res = Object(Type::DICT);
                for(const auto& kv : dict){
                    res.dictSub[kv.first] = load_impl( j[kv.first], dict[kv.first]);
                }
                return res;
            }else if(r.getType() == Type::LIST){
                std::unique_ptr<Rule> list = std::move(r.listSub);
                Object res = Object(Type::LIST);
                for(const auto& v : j.get<std::vector<json>>()){
                    res.listSub.push_back(load_impl( j.at(0), list));
                }
                return res;
            }else{
                if (r.getType() == Type::INT) {
                    return Object(Type::INT, j.get<int>());
                } else if (r.getType() == Type::BOOL) {
                    return Object(Type::BOOL, j.get<bool>());
                } else if (r.getType() == Type::STR) {
                    return Object(Type::STR, j.get<std::string>());
                } else if (r.getType() == Type::FLOAT) {
                    return Object(Type::FLOAT, j.get<float>());
                }
            }

        }

    public:

        std::string dump(Object obj) {
            return dump_impl(obj).dump();
        }

        std::unique_ptr<T> load(std::string s) {
            json data;
            try{
                data = json::parse(s);
            }catch (...){}

            auto rule = T::getJsonParseRule();
            return std::make_unique<T>(load_impl(data,rule));
        }

    };

};
#endif //IROHA_JSON_PARSE_H
