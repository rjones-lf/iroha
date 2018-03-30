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

#include "torii/processor/query_processor_impl.hpp"
#include "backend/protobuf/from_old_model.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"

namespace iroha {
  namespace torii {

    /**
     * Checks if public keys are subset of given signaturies
     * @param signatures user signatories, an iterable collection
     * @param public_keys vector of public keys
     * @return true if user has needed signatories, false instead
     */
    bool signaturesSubset(
        const shared_model::interface::SignatureSetType &signatures,
        const std::vector<shared_model::crypto::PublicKey> &public_keys) {
      // TODO 09/10/17 Lebedev: simplify the subset verification IR-510
      // #goodfirstissue
      // TODO 30/04/2018 x3medima17: remove code duplication in query_processor
      // IR-1192 and stateful_validator
      std::unordered_set<std::string> txPubkeys;
      for (auto sign : signatures) {
        txPubkeys.insert(sign->publicKey().toString());
      }
      return std::all_of(public_keys.begin(),
                         public_keys.end(),
                         [&txPubkeys](const auto &public_key) {
                           return txPubkeys.find(public_key.toString())
                               != txPubkeys.end();
                         });
    }

    /**
     * Builds QueryResponse that contains StatefulError
     * @param hash - original query hash
     * @return QueryRepsonse
     */
    std::shared_ptr<shared_model::interface::QueryResponse> buildStatefulError(
        const shared_model::interface::types::HashType &hash) {
      return clone(
          shared_model::proto::TemplateQueryResponseBuilder<>()
              .queryHash(hash)
              .errorQueryResponse<
                  shared_model::interface::StatefulFailedErrorResponse>()
              .build());
    }

    QueryProcessorImpl::QueryProcessorImpl(
        std::shared_ptr<ametsuchi::Storage> storage)
        : storage_(storage) {}

    bool QueryProcessorImpl::checkSignatories(
        const shared_model::interface::Query &qry) {
      const auto &sig = *qry.signatures().begin();

      const auto &wsv_query = storage_->getWsvQuery();
      auto qpf =
          model::QueryProcessingFactory(wsv_query, storage_->getBlockQuery());
      auto signatories = wsv_query->getSignatories(qry.creatorAccountId());
      if (not signatories) {
        return false;
      }
      bool result = signaturesSubset({sig}, *signatories);
      return result;
    }

    void QueryProcessorImpl::queryHandle(
        std::shared_ptr<shared_model::interface::Query> qry) {
      if (not checkSignatories(*qry)) {
        auto response = buildStatefulError(qry->hash());
        subject_.get_subscriber().on_next(response);
        return;
      }

      const auto &wsv_query = storage_->getWsvQuery();
      auto qpf =
          model::QueryProcessingFactory(wsv_query, storage_->getBlockQuery());
      auto qpf_response = qpf.execute(*qry);
      auto qry_resp =
          std::static_pointer_cast<shared_model::proto::QueryResponse>(
              qpf_response);
      subject_.get_subscriber().on_next(
          std::make_shared<shared_model::proto::QueryResponse>(
              qry_resp->getTransport()));
    }
    rxcpp::observable<std::shared_ptr<shared_model::interface::QueryResponse>>
    QueryProcessorImpl::queryNotifier() {
      return subject_.get_observable();
    }

  }  // namespace torii
}  // namespace iroha
