/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_options.hpp"
#include <boost/algorithm/string.hpp>
#include <regex>

namespace iroha {
  namespace ametsuchi {

    PostgresOptions::PostgresOptions(const std::string &pg_opt)
        : pg_opt_(pg_opt) {
      std::smatch m;
      // regex to fetch dbname from pg_opt string
      std::regex e("\\b(dbname=)([^ ]*)");

      if (std::regex_search(pg_opt_, m, e)) {
        dbname_ = *(m.end() - 1);
        pg_opt_without_db_name_ = m.prefix().str() + m.suffix().str();
      } else {
        dbname_ = boost::none;
        pg_opt_without_db_name_ = pg_opt_;
      }
    }

    std::string PostgresOptions::optionsString() const {
      return pg_opt_;
    }

    std::string PostgresOptions::optionsStringWithoutDbName() const {
      return pg_opt_without_db_name_;
    }

    boost::optional<std::string> PostgresOptions::dbname() const {
      return dbname_;
    }

  }  // namespace ametsuchi
}  // namespace iroha
