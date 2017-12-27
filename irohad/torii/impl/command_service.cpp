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

#include "torii/command_service.hpp"
#include "common/types.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "endpoint.pb.h"

namespace torii {

  CommandService::CommandService(
      std::shared_ptr<iroha::model::converters::PbTransactionFactory>
          pb_factory,
      std::shared_ptr<iroha::torii::TransactionProcessor> txProcessor,
      std::shared_ptr<iroha::ametsuchi::Storage> storage)
      : pb_factory_(pb_factory),
        tx_processor_(txProcessor),
        storage_(storage),
        cache_(std::make_shared<iroha::cache::
                                    Cache<std::string,
                                          iroha::protocol::ToriiResponse>>()) {
    // Notifier for all clients
    tx_processor_->transactionNotifier().subscribe([this](
        std::shared_ptr<iroha::model::TransactionResponse> iroha_response) {
      // Find response in cache
      auto res = cache_->findItem(iroha_response->tx_hash);
      if (not res) {
        iroha::protocol::ToriiResponse response;
        response.set_tx_hash(iroha_response->tx_hash);
        response.set_tx_status(iroha::protocol::NOT_RECEIVED);
        cache_->addItem(iroha_response->tx_hash, response);
        return;
      }
      switch (iroha_response->current_status) {
        case iroha::model::TransactionResponse::STATELESS_VALIDATION_FAILED:
          res->set_tx_status(
              iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED);
          break;
        case iroha::model::TransactionResponse::STATELESS_VALIDATION_SUCCESS:
          res->set_tx_status(
              iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS);
          break;
        case iroha::model::TransactionResponse::STATEFUL_VALIDATION_FAILED:
          res->set_tx_status(
              iroha::protocol::TxStatus::STATEFUL_VALIDATION_FAILED);
          break;
        case iroha::model::TransactionResponse::STATEFUL_VALIDATION_SUCCESS:
          res->set_tx_status(
              iroha::protocol::TxStatus::STATEFUL_VALIDATION_SUCCESS);
          break;
        case iroha::model::TransactionResponse::COMMITTED:
          res->set_tx_status(iroha::protocol::TxStatus::COMMITTED);
          break;
        case iroha::model::TransactionResponse::IN_PROGRESS:
          res->set_tx_status(iroha::protocol::TxStatus::IN_PROGRESS);
          break;
        case iroha::model::TransactionResponse::NOT_RECEIVED:
        default:
          res->set_tx_status(iroha::protocol::TxStatus::NOT_RECEIVED);
          break;
      }

      cache_->addItem(iroha_response->tx_hash, *res);
    });
  }

  void CommandService::ToriiAsync(iroha::protocol::Transaction const &request,
                                  google::protobuf::Empty &empty) {
    auto iroha_tx = pb_factory_->deserialize(request);
    auto tx_hash = iroha::hash(*iroha_tx).to_string();

    if (cache_->findItem(tx_hash)) {
      return;
    }

    iroha::protocol::ToriiResponse response;
    response.set_tx_hash(tx_hash);
    response.set_tx_status(iroha::protocol::TxStatus::IN_PROGRESS);

    cache_->addItem(tx_hash, response);
    // Send transaction to iroha
    tx_processor_->transactionHandle(iroha_tx);
  }

  void CommandService::StatusAsync(
      iroha::protocol::TxStatusRequest const &request,
      iroha::protocol::ToriiResponse &response) {
    auto resp = cache_->findItem(request.tx_hash());
    if (resp) {
      response.CopyFrom(*resp);
    } else {
      response.set_tx_hash(request.tx_hash());
      if (storage_->getBlockQuery()->getTxByHashSync(request.tx_hash())) {
        response.set_tx_status(iroha::protocol::TxStatus::COMMITTED);
      } else {
        response.set_tx_status(iroha::protocol::TxStatus::NOT_RECEIVED);
      }
      cache_->addItem(request.tx_hash(), response);
    }
  }

}  // namespace torii
