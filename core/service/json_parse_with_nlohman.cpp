//
// Created by SonokoMizuki on 2016/11/02.
//
#include "json_parse.hpp"

#include "../consensus/consensus_event.hpp"
#include "../model/objects/asset.hpp"
#include "../model/objects/domain.hpp"
#include "../model/commands/transfer.hpp"
#include "../model/commands/add.hpp"
#include "../model/transaction.hpp"

// ***************8
// *
#include <json.hpp>
// *
// ***************8
namespace json_parse_with_json_nlohman {

    using json = nlohmann::json;

    using Object = json_parse::Object;
    using Rule = json_parse::Rule;
    using Type = json_parse::Type;

    namespace parser {

        namespace detail {
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
                        res = obj.integer;
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
 
            Object load_impl(json& j,const Rule& r){
                if(r.getType() == Type::DICT) {
                    std::map<std::string, Rule> dict = std::move(r.dictSub);
                    Object res = Object(Type::DICT);
                    for(auto& kv : dict){
                        res.dictSub.insert( std::make_pair( kv.first, load_impl( j[kv.first], kv.second)));
                    }
                    return res;
                }else if(r.getType() == Type::LIST){
                    auto list = std::move(r.listSub.at(0));
                    Object res = Object(Type::LIST);
                    for(auto& v : j.get<std::vector<json>>()){
                        res.listSub.push_back(load_impl( v, list));
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
                return Object(Type::INT, -1);
            }
        }

        std::string dump(Object obj) {    
            return detail::dump_impl(obj).dump();
        }

        template<typename T>
        using ConsensusEvent = event::ConsensusEvent<T>;
        template<typename T>
        using Transaction = transaction::Transaction<T>;
        template<typename T>
        using Transfer = command::Transfer<T>;
        template<typename T>
        using Add = command::Add<T>;
        using object::Asset;
        using object::Domain;

        template<>
        std::unique_ptr<
            ConsensusEvent<
                Transaction<
                    Transfer<Asset>
                >
            >
        > load(std::string s) {
            json data;
            try{
                data = json::parse(s);
            }catch (...){}

            auto rule = ConsensusEvent<Transaction<Transfer<Asset>>>::getJsonParseRule();
            return std::make_unique<
                ConsensusEvent<
                    Transaction<
                        Transfer<Asset>
                    >
                >
            >(detail::load_impl(data, rule));
        }

        template<>
        std::unique_ptr<
            ConsensusEvent<
                Transaction<
                    Add<Asset>
                >
            >
        > load(std::string s) {
            json data;
            try{
                data = json::parse(s);
            }catch (...){}

            auto rule = ConsensusEvent<Transaction<Add<Asset>>>::getJsonParseRule();
            return std::make_unique<
                ConsensusEvent<
                    Transaction<
                        Add<Asset>
                    >
                >
            >(detail::load_impl(data, rule));
        }

        template<>
        std::unique_ptr<
            ConsensusEvent<
                Transaction<
                    Add<Domain>
                >
            >
        > load(std::string s) {
            json data;
            try{
                data = json::parse(s);
            }catch (...){}

            auto rule = ConsensusEvent<Transaction<Add<Domain>>>::getJsonParseRule();
            return std::make_unique<
                ConsensusEvent<
                    Transaction<
                        Add<Domain>
                    >
                >
            >(detail::load_impl(data, rule));
        }

    };

};