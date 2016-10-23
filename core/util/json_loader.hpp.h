//
// Created by SonokoMizuki on 2016/10/22.
//

#ifndef IROHA_JSON_LOADER_HPP_H
#define IROHA_JSON_LOADER_HPP_H

#include <json.hpp>

namespace util{
    class JsonLoader{
    public:
        JsonLoader(std::string filename);

        template<typename T>
        T get(std::string key);

        bool exist(std::string key);

    };
};

#endif //IROHA_JSON_LOADER_HPP_H
