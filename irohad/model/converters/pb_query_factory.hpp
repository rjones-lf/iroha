/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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
#ifndef IROHA_PB_QUERY_FACTORY_HPP
#define IROHA_PB_QUERY_FACTORY_HPP

#include <typeindex>
#include <unordered_map>
#include "logger/logger.hpp"
#include "model/common.hpp"
#include "model/query.hpp"
#include "queries.pb.h"

namespace iroha {
  namespace model {
    namespace converters {

      /**
       * Converting business objects to protobuf and vice versa
       */
      class PbQueryFactory {
       public:
        /**
         * Convert proto query to model query
         * @param pb_block - reference to proto query
         * @return model Query
         */
        optional_ptr<model::Query> deserialize(
            const protocol::Query &pb_query) const;

        /**
         * Convert model query to proto query
         * @param query - model query to serialize
         * @return boost::noneif no query type is found
         */
        boost::optional<protocol::Query> serialize(
            std::shared_ptr<const model::Query> query) const;

        PbQueryFactory();

       private:
        // Query serializer:
        protocol::Query serializeGetAccount(
            std::shared_ptr<const Query> query) const;
        protocol::Query serializeGetAccountAssets(
            std::shared_ptr<const Query> query) const;
        protocol::Query serializeGetAccountDetail(
            std::shared_ptr<const Query> query) const;
        protocol::Query serializeGetTransactions(
            std::shared_ptr<const Query> query) const;
        protocol::Query serializeGetAccountTransactions(
            std::shared_ptr<const Query> query) const;
        protocol::Query serializeGetAccountAssetTransactions(
            std::shared_ptr<const Query> query) const;
        protocol::Query serializeGetSignatories(
            std::shared_ptr<const Query> query) const;
        protocol::Query serializeGetAssetInfo(
            std::shared_ptr<const Query> query) const;
        protocol::Query serializeGetRoles(
            std::shared_ptr<const Query> query) const;
        protocol::Query serializeGetRolePermissions(
            std::shared_ptr<const Query> query) const;

        /**
         * Serialize and add meta data of model query to proto query
         * @param pb_query - protocol query  object
         * @param query - model query to serialize
         */
        void serializeQueryMetaData(protocol::Query &pb_query,
                                    std::shared_ptr<const Query> query) const;

        using Serializer = protocol::Query (PbQueryFactory::*)(
            std::shared_ptr<const Query>) const;
        std::unordered_map<std::type_index, Serializer> serializers_;

        logger::Logger log_;
      };

    }  // namespace converters
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_PB_QUERY_FACTORY_HPP
