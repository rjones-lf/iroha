#ifndef IROHA_JSON_PARSE_H
#define IROHA_JSON_PARSE_H

#include <string>
#include <map>
#include <memory>
#include <vector>

namespace json_parse {

    enum class Type {
        BOOL,
        STR,
        INT,
        FLOAT,
        LIST,
        DICT,
        INVELIED
    };

    struct Object {
        const Type type;
        int integer = -1;
        std::string str = "";
        bool boolean = false;
        float floating = 0.0f;

        std::map<std::string, Object> dictSub;
        std::vector<Object> listSub;

        Object(const Type& t,const int& i) :
            type(std::move(t)), integer(std::move(i))
        {
            if(t != Type::INT) throw "TypeError"; //WIP
        }

        Object(const Type& t,const std::string& s) :
            type(t), str(s)
        {
            if(t != Type::STR) throw "TypeError"; //WIP
        }

        Object(const Type& t,const bool& s) :
            type(t), boolean(s)
        {
            if(t != Type::BOOL) throw "TypeError"; //WIP
        }

        Object(const Type& t,const float& f) :
                type(t), floating(f)
        {
            if(t != Type::FLOAT) throw "TypeError"; //WIP
        }

        Object():type(Type::INVELIED){};

        Object(Type t) :
            type(t)
        {}

        Type getType() const {
            return type;
        }

    };

    struct Rule;
    struct Rule {
        Type type;

        Rule(Type t):
            type(t)
        {};

        Type getType() const {
            return type;
        }
        std::map<std::string, Rule> dictSub;
        std::vector<Rule> listSub;

        operator std::string() const { 
            std::string res;
            switch(type){
                case Type::BOOL: return "bool";
                case Type::STR:  return "string";
                case Type::INT:  return "int";
                case Type::FLOAT:return "float";
                case Type::LIST:
                    return "[" + std::string(listSub.at(0)) + "]";
                case Type::DICT:
                    res = "{";
                    for(auto&& s : dictSub){
                        res += "\"" + s.first + "\":" + std::string(s.second) + ",";
                    }
                    res += "}";
                    return res;
                case Type::INVELIED:
                    return "invalied";
            }    
        }
    };
};
#endif //IROHA_JSON_PARSE_H
