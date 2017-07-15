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

#include <gtest/gtest.h>
#include <consensus/consensus_client.hpp>
#include <consensus/consensus_service.hpp>
#include <consensus/messages.hpp>
#include <future>
#include <uvw/loop.hpp>
#include <grpc++/grpc++.h>
#include <uvw.hpp>

TEST(GRPC, ConsensusStubsConnection) {
  auto loop = uvw::Loop::getDefault();

  bool proposal_received = false;

  std::thread([&proposal_received]() {
    iroha::ConsensusService stub;

    stub.once<const iroha::consensus::Proposal*>([&proposal_received](auto &p, auto &s) {
      printf("recv\n");
      proposal_received = true;
    });

    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:65530", grpc::InsecureServerCredentials());
    builder.RegisterService(&stub);
    auto server(builder.BuildAndStart());

    server->Wait();
  }).detach();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  iroha::ConsensusClient client("0.0.0.0", 65530);

  iroha::Proposal proposal;
  client.SendProposal(&proposal);

  // non-blocking
  loop->run<uvw::Loop::Mode::ONCE>();

  ASSERT_TRUE(proposal_received);
}