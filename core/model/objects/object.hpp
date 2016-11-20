//
// Created by SonokoMizuki on 2016/11/01.
//

#ifndef IROHA_OBJECTS_H
#define IROHA_OBJECTS_H

#include "../../service/json_parse.hpp"

// Abstract
class AbsObject {
    virtual json_parse::Object dump() = 0;
    virtual json_parse::Rule getJsonParseRule() = 0;
};

#endif //IROHA_OBJECTS_H
