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

#ifndef __CORE_REPOSITORY_DOMAIN_SAMPLE_ASSET_REPOSITORY_HPP__
#define __CORE_REPOSITORY_DOMAIN_SAMPLE_ASSET_REPOSITORY_HPP__

#include <string>
#include <memory>
#include <vector>

namespace repository {
    namespace domain {

        template<typename T>
        bool add(std::string key, T value);

        template<typename T>
        bool update(std::string key, T newValue);

        // This is OK...??
        template<typename T>
        bool remove(std::string key);

        template<typename T>
        std::vector<std::unique_ptr<T> > findAll(std::string key);

        template<typename T>
        std::unique_ptr<T> findOne(std::string key);

        template<typename T>
        std::unique_ptr<T> findOrElse(std::string key, T defaultVale);

        template<typename T>
        bool isExist(std::string key);
    }

};
#endif  // CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_
