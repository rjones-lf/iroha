// Created by SonokoMizuki on 2016/11/01.
//

#ifndef IROHA_COMMAND_H
#define IROHA_COMMAND_H

#include "../../service/json_parse.hpp"

namespace command {

// Abstract　
class Command {
public:

    virtual std::string getCommandName() const = 0;

    using Object = json_parse::Object;
    using Rule = json_parse::Rule;
    using Type = json_parse::Type;

    virtual Object dump() = 0;
    virtual Rule getJsonParseRule() = 0;
    virtual ~Command(){};
};

};
#endif //IROHA_COMMAND_H
