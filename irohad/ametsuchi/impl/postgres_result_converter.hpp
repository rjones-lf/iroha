/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_POSTGRES_RESULT_CONVERTER_HPP
#define IROHA_POSTGRES_RESULT_CONVERTER_HPP

#include "interfaces/common_objects/common_objects_factory.hpp"

namespace iroha {
  namespace ametsuchi {
    /**
     * This class is used to convert result set obtained from postgres to
     * iroha common objects
     */
    class PostgresResultConverter {
     public:
      explicit PostgresResultConverter(
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              factory) {}

      template <typename T>
      using ConverterResult =
          shared_model::interface::CommonObjectsFactory::FactoryResult<T>;

      ConverterResult<std::unique_ptr<shared_model::interface::Account>>
      createAccount(const pqxx::row &row);

      ConverterResult<std::unique_ptr<shared_model::interface::Asset>>
      createAsset(const pqxx::row &row);

      ConverterResult<std::unique_ptr<shared_model::interface::AccountAsset>>
      createAccountAsset(const pqxx::row &row);

      ConverterResult<
          std::vector<std::unique_ptr<shared_model::interface::AccountAsset>>>
      createAccountAssets(const pqxx::result &result);

      ConverterResult<std::unique_ptr<shared_model::interface::Peer>>
      createPeer(const pqxx::row &row);

      ConverterResult<std::vector<std::unique_ptr<shared_model::interface::Peer>>>
      createPeers(const pqxx::result &result);

      ConverterResult<std::unique_ptr<shared_model::interface::Domain>>
      createDomain(const pqxx::row &row);

     private:
      std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory_;
    };  // namespace ametsuchi
  }     // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_RESULT_CONVERTER_HPP
