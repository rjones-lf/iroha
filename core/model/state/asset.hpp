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

#ifndef CORE_DOMAIN_ASSET_HPP_
#define CORE_DOMAIN_ASSET_HPP_

#include <string>
#include "../../util/random.hpp"

namespace domain {
    namespace asset {

      class Asset {
        public:
          std::string name;
          std::string parentDomainName;
          std::string uid;

          // Constructor is only used by factory.
          Asset(
            std::string aName,
            std::string aParentDomainName
          ):
            name(aName),
            parentDomainName(aParentDomainName),
            uid(random_service::makeRandomHash())
          {}
  
          virtual ~Asset() = default;

          // Support move and copy.
          Asset(Asset const&) = default;
          Asset(Asset&&) = default;
          Asset& operator =(Asset const&) = default;
          Asset& operator =(Asset&&) = default;
      };

    };  // namespace asset
};  // namespace domain

#endif