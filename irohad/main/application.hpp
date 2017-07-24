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
#ifndef IROHA_APPLICATION_HPP
#define IROHA_APPLICATION_HPP

#include <main/context.hpp>
#include <network/peer_communication_service.hpp>
#include <consensus/consensus_service_stub.hpp>
#include <torii/processor/query_processor_stub.hpp>
#include <torii/processor/transaction_processor_impl.hpp>
#include <validation/chain/validator_stub.hpp>
#include <validation/stateless/validator_impl.hpp>

#include <model/model_crypto_provider_impl.hpp>
#include <crypto/crypto.hpp>
#include <ametsuchi/impl/storage_impl.hpp>

#include <main/server_runner.hpp>


class Irohad{

  public:
    std::shared_ptr<Context> context;

    Irohad();

    void run();

};

#endif //IROHA_APPLICATION_HPP