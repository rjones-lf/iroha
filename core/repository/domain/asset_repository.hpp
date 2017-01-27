/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

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
#ifndef IROHA_ASSET_REPOSITORY_H
#define IROHA_ASSET_REPOSITORY_H

namespace repository {
    namespace asset {

        bool add(std::string publicKey, std::string assetName, std::string value);
        bool update(std::string publicKey, std::string assetName, std::string newValue);
        bool remove(std::string publicKey, std::string assetName);
        std::vector <std::string> findAll(std::string key);

        std::string findOne(std::string key);

        std::string findOrElse(std::string key, std::string defaultVale);

        bool isExist(std::string key);
    };
};


#endif //IROHA_ASSET_REPOSITORY_H
