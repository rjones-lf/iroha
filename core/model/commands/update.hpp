/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef IROHA_UPDATE_H
#define IROHA_UPDATE_H

#include "../objects/domain.hpp"
#include "../objects/asset.hpp"
#include "../objects/message.hpp"
#include "../objects/peer.hpp"

#include <iostream>
#include "../../util/logger.hpp"

namespace command {

    template <typename T>
    class Update: public T {

    public:
        std::string ownerPublicKey;

        template<typename... Args>
        explicit Update(
            std::string&& ownerPublicKey,
            Args&&... args
        ):
            T(std::forward<Args>(args)...),
            ownerPublicKey(std::move(ownerPublicKey))
        {}

        auto getCommandName() const {
            return "Update";
        }

        void execution();

    };

};  // namespace command

#endif //IROHA_UPDATE_H
