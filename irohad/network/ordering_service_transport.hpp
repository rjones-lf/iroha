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
#ifndef IROHA_ORDERING_SERVICE_TRANSPORT_H
#define IROHA_ORDERING_SERVICE_TRANSPORT_H

#include "ordering.grpc.pb.h"
#include "model/proposal.hpp"
#include "ordering.grpc.pb.h"

namespace iroha {
  namespace network {

    class OrderingServiceTransport {

    public:

      virtual void publishProposal(std::shared_ptr<ordering::proto::Proposal>,
                                   const std::unique_ptr<ordering::proto::OrderingGate::Stub>& ) = 0;


      virtual ~OrderingServiceTransport() = default;
    };

  }
}

#endif //IROHA_ORDERING_SERVICE_TRANSPORT_H
