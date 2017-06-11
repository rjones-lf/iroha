/*
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *      http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <asset_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <main_generated.h>
#include <endpoint_generated.h>
#include <primitives_generated.h>
#include <endpoint.grpc.fb.h>
#include <membership_service/peer_service.hpp>
#include <service/flatbuffer_service.h>
#include <crypto/signature.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <service/connection.hpp>

#include <grpc++/grpc++.h>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

using grpc::Channel;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ClientContext;
using grpc::Status;


using ConsensusEvent = iroha::ConsensusEvent;
using Transaction = iroha::Transaction;

class connection_with_grpc_flatbuffer_test : public testing::Test {
 protected:

  void runServer() {
    connection::iroha::SumeragiImpl::Verify::receive(
        [](const std::string& from, flatbuffers::unique_ptr_t&& u) {
          auto event  = flatbuffers::GetRoot<ConsensusEvent>(u.get());
          auto txroot = event->transactions()->Get(0)->tx_nested_root();
          std::cout << "Verify::receive\n";
          std::cout << flatbuffer_service::toString(*txroot) << std::endl;
        });

    connection::iroha::SumeragiImpl::Torii::receive(
        [](const std::string& from, flatbuffers::unique_ptr_t&& tx) {
          auto txroot =
              flatbuffers::GetRoot<::iroha::Transaction>(tx.get());
          std::cout << flatbuffer_service::toString(*txroot) << std::endl;
        });

    connection::iroha::AssetRepositoryImpl::AccountGetAsset::receive([=](
            const std::string & /* from */, flatbuffers::unique_ptr_t &&query_ptr) -> std::vector<const ::iroha::Asset *>{
        const iroha::AssetQuery& query = *flatbuffers::GetRoot<iroha::AssetQuery>(query_ptr.get());
        flatbuffers::FlatBufferBuilder fbb;

        EXPECT_STREQ(query.pubKey()->c_str(),     "my_pubkey");
        EXPECT_STREQ(query.ledger_name()->c_str(),"req_ledger_name");
        EXPECT_STREQ(query.domain_name()->c_str(),"req_domain_name");
        EXPECT_STREQ(query.asset_name()->c_str() ,"req_asset_name");

        auto res_asset = iroha::CreateAsset(
              fbb,iroha::AnyAsset::Currency,
              iroha::CreateCurrencyDirect(
                  fbb,"sample","my_domain","my_ledger", "my_description", "my_amount", 0
              ).Union()
        );
        fbb.Finish(res_asset);
        std::vector<const ::iroha::Asset *> res{
            flatbuffers::GetMutableRoot<::iroha::Asset>(fbb.GetBufferPointer())
        };
        return res;
    });

    connection::run();
  }

  std::thread serverThread;

  virtual void SetUp() {
    logger::setLogLevel(logger::LogLevel::Debug);
    serverThread = std::thread(&connection_with_grpc_flatbuffer_test::runServer, this);
    connection::waitForReady();
  }

  virtual void TearDown() {
    connection::finish();
    serverThread.join();
  }
};

TEST_F(connection_with_grpc_flatbuffer_test, Transaction_Add_Asset) {
  flatbuffers::FlatBufferBuilder xbb;

  const auto assetBuf = flatbuffer_service::asset::CreateCurrency(
      "IROHA", "Domain", "Ledger", "Desc", "31415", 4);

  const auto add = ::iroha::CreateAddDirect(xbb, "AccPubKey", &assetBuf);

  std::vector<flatbuffers::Offset<iroha::Signature>> signatures;
  std::vector<uint8_t> blob = {'a', 'b', 'c', 'd'};
  signatures.push_back(
      iroha::CreateSignatureDirect(xbb, "SIG'S PUBKEY", &blob, 9999));

  std::vector<uint8_t> hash = {'H', 'S'};
  std::vector<uint8_t> data = {'D', 'T'};

  const auto stamp = 9999999;
  const auto attachment = iroha::CreateAttachmentDirect(xbb, "MIME", &data);
  const auto txoffset = iroha::CreateTransactionDirect(
      xbb, "TX'S CREATOR", iroha::Command::Add,
      add.Union(), &signatures, &hash, stamp, attachment);
  xbb.Finish(txoffset);

  auto txflatbuf = xbb.ReleaseBufferPointer();
  auto txptr = flatbuffers::GetRoot<Transaction>(txflatbuf.get());

  auto event = flatbuffer_service::toConsensusEvent(*txptr);
  ASSERT_TRUE(event);
  flatbuffers::unique_ptr_t uptr;
  event.move_value(uptr);
  auto eventptr = flatbuffers::GetRoot<ConsensusEvent>(uptr.get());
  connection::iroha::SumeragiImpl::Verify::send(
      config::PeerServiceConfig::getInstance().getMyIp(), *eventptr);
}

TEST_F(connection_with_grpc_flatbuffer_test, AssetRepository_serverAccountGetAsset) {

  auto channel = grpc::CreateChannel(
     "0.0.0.0:50051",
     grpc::InsecureChannelCredentials()
  );

  auto stub = iroha::AssetRepository::NewStub(channel);
  grpc::ClientContext context;

  flatbuffers::FlatBufferBuilder fbb;
  auto query_offset = iroha::CreateAssetQueryDirect(
        fbb,
        "my_pubkey",
        "req_ledger_name",
        "req_domain_name",
        "req_asset_name",
        true
  );
  fbb.Finish(query_offset);
  auto request = flatbuffers::BufferRef<iroha::AssetQuery>(
        fbb.GetBufferPointer(),fbb.GetSize()
  );
  flatbuffers::BufferRef<iroha::AssetResponse> response;
  auto status = stub->AccountGetAsset(&context, request, &response);

  ASSERT_TRUE(status.ok());

  auto res = response.GetRoot();

  ASSERT_EQ(res->assets()->Length(), 1);
  ASSERT_EQ(res->assets()->Get(0)->asset_type(), iroha::AnyAsset::Currency);
  ASSERT_STREQ(
     res->assets()->Get(0)->asset_as_Currency()->currency_name()->c_str(),
     "sample"
  );
  ASSERT_STREQ(
      res->assets()->Get(0)->asset_as_Currency()->domain_name()->c_str(),
      "my_domain"
  );
  ASSERT_STREQ(
      res->assets()->Get(0)->asset_as_Currency()->ledger_name()->c_str(),
      "my_ledger"
  );
  ASSERT_STREQ(
      res->assets()->Get(0)->asset_as_Currency()->description()->c_str(),
      "my_description"
  );
  ASSERT_STREQ(
      res->assets()->Get(0)->asset_as_Currency()->amount()->c_str(),
      "my_amount"
  );
  ASSERT_EQ(
      res->assets()->Get(0)->asset_as_Currency()->precision(),
      0
  );

}
