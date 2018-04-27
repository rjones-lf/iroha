/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_POSTGRES_OPTIONS_HPP
#define IROHA_POSTGRES_OPTIONS_HPP

#include <boost/optional.hpp>
#include <unordered_map>
#include "common/result.hpp"

namespace iroha {
  namespace ametsuchi {

    class PostgresOptions {
     public:
      static expected::Result<PostgresOptions, std::string> create(
          const std::string pg_opt);
      PostgresOptions() = delete;

      std::string optionsString() const;
      std::string optionsStringWithoutDbName() const;
      boost::optional<std::string> getOption(const std::string &option) const;

     private:
      PostgresOptions(
          const std::string &pg_opt,
          const std::string &pg_opt_without_db_name,
          const std::unordered_map<std::string, std::string> &options_map);

      const std::string pg_opt_;
      const std::string pg_opt_without_db_name_;
      const std::unordered_map<std::string, std::string> options_map_;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_OPTIONS_HPP
