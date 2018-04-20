/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_GRPC_CHANNEL_BUILDER_HPP
#define IROHA_GRPC_CHANNEL_BUILDER_HPP

#include <grpc++/grpc++.h>

namespace iroha {
  namespace network {

    /**
     * Creates client which is capable of sending and receiving
     * messages of INT_MAX bytes size
     * @tparam T type for gRPC stub, e.g. proto::Yac
     * @param address ip address for connection, ipv4:port
     * @return gRPC stub of parametrized type
     */
    template <typename T>
    auto createClient(const grpc::string& address) {
      // in order to bypass built-in limitation of gRPC message size
      grpc::ChannelArguments args;
      args.SetMaxSendMessageSize(INT_MAX);
      args.SetMaxReceiveMessageSize(INT_MAX);

      return
          T::NewStub(grpc::CreateCustomChannel(
              address, grpc::InsecureChannelCredentials(), args));
    }
  } // namespace network
} // namespace iroha

#endif  // IROHA_GRPC_CHANNEL_BUILDER_HPP
