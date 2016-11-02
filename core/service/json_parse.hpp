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
        DICT
    };

    struct Object {
        Type type;
        int integer;
        std::string str;
        bool boolean;
        float floating;

        Object(Type t,int i) :
            type(t), integer(i)
        {
            if(t != Type::INT) throw "TypeError"; //WIP
        }

        Object(Type t,std::string s) :
            type(t), str(s)
        {
            if(t != Type::STR) throw "TypeError"; //WIP
        }

        Object(Type t,bool s) :
            type(t), boolean(s)
        {
            if(t != Type::BOOL) throw "TypeError"; //WIP
        }

        Object(Type t,float f) :
                type(t), floating(f)
        {
            if(t != Type::FLOAT) throw "TypeError"; //WIP
        }


        Object(Type t) :
                type(t) {}

        Type getType() const {
            return type;
        }

        std::map<std::string, Object> dictSub;
        std::vector<Object> listSub;
    };

    struct Rule {
        Type type;

        Rule(Type t):
            type(t)
        {};

        Type getType() const {
            return type;
        }
        std::map<std::string, Rule> dictSub;
        std::unique_ptr<Rule> listSub;
    };
};
#endif //IROHA_JSON_PARSE_H
