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

#ifndef __CORE_REPOSITORY_DOMAIN_COMMON_REPOSITORY_HPP__
#define __CORE_REPOSITORY_DOMAIN_COMMON_REPOSITORY_HPP__

#include <infra/protobuf/api.pb.h>
#include <string>
#include <transaction_builder/transaction_builder.hpp>
#include <vector>

namespace repository {
  namespace common {

/********************************************************************************************
 * stringify / parse
 ********************************************************************************************/

    struct Prefix {
      explicit Prefix(const char *rhs) : prefix_(rhs) {}

      const std::string &operator*() const { return prefix_; }

      std::string prefix_;
    };

    inline void assertPrefix(const std::string &str, const Prefix &prefix) {
      IROHA_ASSERT_TRUE(str.size() > (*prefix).size());
      IROHA_ASSERT_TRUE(*prefix == str.substr(0, (*prefix).size()));
    }

    inline std::string eliminatePrefix(const std::string &str, const Prefix &prefix) {
      assertPrefix(str, prefix);
      return str.substr((*prefix).size());
    }

    template<typename ApiObjectT>
    std::string stringify(const ApiObjectT &obj, const Prefix &prefix) {
      std::string str;
      obj.SerializeToString(&str);
      return *prefix + str;
    }

    template<typename ApiObjectT>
    ApiObjectT parse(const std::string &str_, const Prefix &prefix) {
      const auto str = eliminatePrefix(str_, prefix);
      ApiObjectT ret;
      ret.ParseFromString(str);
      return ret;
    }
  }
}
#endif // __CORE_REPOSITORY_DOMAIN_COMMON_REPOSITORY_HPP__