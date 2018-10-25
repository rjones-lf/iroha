/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_sig_transactions/transport/mst_transport_grpc.hpp"

#include <gtest/gtest.h>
#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"
#include "backend/protobuf/proto_transport_factory.hpp"
#include "interfaces/iroha_internal/transaction_batch_factory_impl.hpp"
#include "interfaces/iroha_internal/transaction_batch_parser_impl.hpp"
#include "module/irohad/multi_sig_transactions/mst_mocks.hpp"
#include "module/irohad/multi_sig_transactions/mst_test_helpers.hpp"
#include "module/shared_model/validators/validators.hpp"
#include "multi_sig_transactions/state/mst_state.hpp"
#include "validators/field_validator.hpp"

using namespace iroha::network;
using namespace iroha::model;

using ::testing::_;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;

/**
 * @brief Sends data over MstTransportGrpc (MstState and Peer objects) and
 * receives them. When received deserializes them end ensures that deserialized
 * objects equal to objects before sending.
 *
 * @given Initialized transport
 * AND MstState for transfer
 * @when Send state via transport
 * @then Assume that received state same as sent
 */
TEST(TransportTest, SendAndReceive) {
  auto async_call_ = std::make_shared<
      iroha::network::AsyncGrpcClient<google::protobuf::Empty>>();
  auto tx_validator = std::make_unique<shared_model::validation::MockValidator<
      shared_model::interface::Transaction>>();
  auto tx_factory = std::make_shared<shared_model::proto::ProtoTransportFactory<
      shared_model::interface::Transaction,
      shared_model::proto::Transaction>>(std::move(tx_validator));
  auto parser =
      std::make_shared<shared_model::interface::TransactionBatchParserImpl>();
  auto batch_factory =
      std::make_shared<shared_model::interface::TransactionBatchFactoryImpl>();
  auto my_key = makeKey();
  auto transport = std::make_shared<MstTransportGrpc>(async_call_,
                                                      std::move(tx_factory),
                                                      std::move(parser),
                                                      std::move(batch_factory),
                                                      my_key.publicKey());
  auto notifications = std::make_shared<iroha::MockMstTransportNotification>();
  transport->subscribe(notifications);

  std::mutex mtx;
  std::condition_variable cv;

  auto time = iroha::time::now();
  auto state = iroha::MstState::empty();
  state += addSignaturesFromKeyPairs(
      makeTestBatch(txBuilder(1, time)), 0, makeKey());
  state += addSignaturesFromKeyPairs(
      makeTestBatch(txBuilder(2, time)), 0, makeKey());
  state += addSignaturesFromKeyPairs(
      makeTestBatch(txBuilder(3, time)), 0, makeKey());
  state += addSignaturesFromKeyPairs(
      makeTestBatch(txBuilder(3, time)), 0, makeKey());

  ASSERT_EQ(3, state.getBatches().size());

  std::unique_ptr<grpc::Server> server;

  grpc::ServerBuilder builder;
  int port = 0;
  std::string addr = "localhost:";
  builder.AddListeningPort(
      addr + "0", grpc::InsecureServerCredentials(), &port);
  builder.RegisterService(transport.get());
  server = builder.BuildAndStart();
  ASSERT_TRUE(server);
  ASSERT_NE(port, 0);

  std::shared_ptr<shared_model::interface::Peer> peer =
      makePeer(addr + std::to_string(port), "abcdabcdabcdabcdabcdabcdabcdabcd");
  // we want to ensure that server side will call onNewState()
  // with same parameters as on the client side
  EXPECT_CALL(*notifications, onNewState(_, _))
      .WillOnce(Invoke(
          [&my_key, &cv, &state](const auto &from_key, auto const &target_state) {
            EXPECT_EQ(my_key.publicKey(), from_key);

            EXPECT_EQ(state, target_state);
            cv.notify_one();
          }));

  transport->sendState(*peer, state);
  std::unique_lock<std::mutex> lock(mtx);
  cv.wait_for(lock, std::chrono::milliseconds(5000));

  server->Shutdown();
}
