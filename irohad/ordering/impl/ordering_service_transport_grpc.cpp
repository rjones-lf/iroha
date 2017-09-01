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
#include "ordering_service_transport_grpc.hpp"


using namespace iroha::ordering;
using namespace iroha;

void OrderingServiceTransportGrpc::publishProposal(std::shared_ptr<proto::Proposal> proposal,
                                                   const std::unique_ptr<proto::OrderingGate::Stub>& peer) {

  auto call = new AsyncClientCall;

  call->response_reader =
          peer->AsyncSendProposal(&call->context, *proposal, &cq_);

  call->response_reader->Finish(&call->reply, &call->status, call);
}
