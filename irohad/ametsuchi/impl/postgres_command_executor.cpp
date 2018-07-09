/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_command_executor.hpp"

#include <boost/format.hpp>

namespace iroha {
  namespace ametsuchi {

    PostgresCommandExecutor::PostgresCommandExecutor(pqxx::nontransaction &transaction)
        : transaction_(transaction),
          execute_{makeExecuteResult(transaction_)} {}

    WsvCommandResult PostgresCommandExecutor::addAssetQuantity(
        const shared_model::interface::types::AccountIdType &account_id,
        const shared_model::interface::types::AssetIdType &asset_id,
        const std::string &amount,
        const shared_model::interface::types::PrecisionType precision) {
      std::string query = (boost::format(
                               // clang-format off
          R"(
          WITH has_account AS (SELECT account_id FROM account WHERE account_id = '%s' LIMIT 1),
               has_asset AS (SELECT asset_id FROM asset WHERE asset_id = '%s' AND precision = %d LIMIT 1),
               amount AS (SELECT amount FROM account_has_asset WHERE asset_id = '%s' AND account_id = '%s' LIMIT 1),
               new_value AS (SELECT %s +
                              (SELECT
                                  CASE WHEN EXISTS (SELECT amount FROM amount LIMIT 1) THEN (SELECT amount FROM amount LIMIT 1)
                                  ELSE 0::decimal
                              END) AS value
                          ),
               inserted AS
               (
                  INSERT INTO account_has_asset(account_id, asset_id, amount)
                  (
                      SELECT '%s', '%s', value FROM new_value
                      WHERE EXISTS (SELECT * FROM has_account LIMIT 1) AND
                        EXISTS (SELECT * FROM has_asset LIMIT 1) AND
                        EXISTS (SELECT value FROM new_value WHERE value < 2 ^ 253 - 1 LIMIT 1)
                  )
                  ON CONFLICT (account_id, asset_id) DO UPDATE SET amount = EXCLUDED.amount
                  RETURNING (1)
               )
          SELECT CASE
              WHEN EXISTS (SELECT * FROM inserted LIMIT 1) THEN 0
              WHEN NOT EXISTS (SELECT * FROM has_account LIMIT 1) THEN 1
              WHEN NOT EXISTS (SELECT * FROM has_asset LIMIT 1) THEN 2
              WHEN NOT EXISTS (SELECT value FROM new_value WHERE value < 2 ^ 253 - 1 LIMIT 1) THEN 3
              ELSE 4
          END AS result;)"
                               // clang-format on
                               )
                           % account_id % asset_id % ((int)precision) % asset_id
                           % account_id % amount % account_id % asset_id)
                              .str();

      auto result = execute_(query);

      return result.match(
          [](expected::Value<pqxx::result> &error_code) -> WsvCommandResult {
            int code = error_code.value[0].at("result").template as<int>();
            if (code == 0) {
              return {};
            }
            std::string error_msg;
            switch (code) {
              case 1:
                error_msg = "Account does not exist";
                break;
              case 2:
                error_msg = "Asset with given precision does not exist";
                break;
              case 3:
                error_msg = "Summation overflows uint256.";
                break;
              default:
                error_msg = "Unexpected error. Something went wrong!";
                break;
            }
            return expected::makeError(error_msg);
          },
          [](const auto &e) -> WsvCommandResult {
            return expected::makeError(e.error);
          });
    }
  }  // namespace ametsuchi
}  // namespace iroha
