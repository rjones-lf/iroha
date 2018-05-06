/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_options.hpp"
#include <boost/algorithm/string.hpp>
#include <regex>

namespace iroha {
  namespace ametsuchi {

    expected::Result<PostgresOptions, std::string> PostgresOptions::create(
        std::string postgres_options) {
      // Split into param=value list (split by space)
      std::regex pattern{R"([^ ]+)"};
      std::set<std::string> tokens{
          std::sregex_token_iterator{std::begin(postgres_options),
                                     std::end(postgres_options),
                                     pattern},
          std::sregex_token_iterator{}};
      std::unordered_map<std::string, std::string> options;
      std::string pg_opt_without_db_name;
      for (const auto &s : tokens) {
        std::vector<std::string> key_value;
        boost::split(key_value, s, boost::is_any_of("="));
        if (key_value.size() != 2) {
          return expected::makeError("postgres options parse error: cannot get param name and value from " + s);
        }
        std::string key = key_value.at(0);
        std::string value = key_value.at(1);
        if (key.empty() or value.empty()) {
          return expected::makeError("postgres options parse error: param name or value is empty");
        }
        options.insert({key, value});
        if (key != "dbname") {
          pg_opt_without_db_name += s + " ";
        }
      }
      return expected::makeValue(
          PostgresOptions(postgres_options, pg_opt_without_db_name, options));
    }

    PostgresOptions::PostgresOptions(
        const std::string &pg_opt,
        const std::string &pg_opt_without_db_name,
        const std::unordered_map<std::string, std::string> &options_map)
        : pg_opt_(pg_opt),
          pg_opt_without_db_name_(pg_opt_without_db_name),
          options_map_(options_map) {}

    std::string PostgresOptions::optionsString() const {
      return pg_opt_;
    }

    std::string PostgresOptions::optionsStringWithoutDbName() const {
      return pg_opt_without_db_name_;
    }

    boost::optional<std::string> PostgresOptions::getOption(
        const std::string &option) const {
      return options_map_.find(option) != options_map_.end()
          ? boost::make_optional(options_map_.at(option))
          : boost::none;
    }

  }  // namespace ametsuchi
}  // namespace iroha
