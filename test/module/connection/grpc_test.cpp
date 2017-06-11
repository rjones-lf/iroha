/**
 * Copyright 2016 Soramitsu Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <grpc++/grpc++.h>
#include <gtest/gtest.h>
#include <endpoint_generated.h>
#include <account_generated.h>
#include <endpoint.grpc.fb.h>
#include <memory>
#include <assert.h>
#include <thread>
#include <flatbuffers/flatbuffers.h>
#include "../../../tools/tx_generator.h"

template <typename T>
using Offset = flatbuffers::Offset<T>;

inline std::vector<uint8_t> toVector(const std::string& s) {
  std::vector<uint8_t> ret;
  for (auto& e: s) { ret.push_back(e); }
  return ret;
}


using Block            = ::protocol::Block;
using Transaction      = ::protocol::Transaction;
using SumeragiResponse = ::protocol::SumeragiResponse;

template <typename T>
using Offset = flatbuffers::Offset<T>;

// Track the server instance, so we can terminate it later.
grpc::Server *server_instance = nullptr;
// Mutex to protec this variable.
std::mutex wait_for_server;
std::condition_variable server_instance_cv;

bool noDebugLog;

void Log(std::string const& s) {
  if (!noDebugLog)
    std::cout << s << "\n";
}

/**
 * This is not real SumeragiService
 * This class is intended to test gRPC.
 */
class SumeragiServiceImpl : public ::protocol::Sumeragi::Service {

  virtual ::grpc::Status Torii(
    ::grpc::ServerContext *context,
    const flatbuffers::BufferRef<Transaction> *request,
    flatbuffers::BufferRef<SumeragiResponse> *response) {

    // Guarantee no throw.
    /* std::cout << */ dump::toString(*request->GetRoot()) /* << std::endl */;

    /* ----------------------------------------------------
     * Create SumeragiResponse
     * -------------------------------------------------- */
    fbb.Clear();
    auto pk = toVector("PK");
    auto sg = toVector("SG");
    auto respOffset = protocol::CreateSumeragiResponse(
      fbb,
      fbb.CreateString("OK"),
      protocol::ResponseCode::OK,
      protocol::CreateSignatureDirect(
        fbb, &pk, &sg
      )
    );
    fbb.Finish(respOffset);
    *response = flatbuffers::BufferRef<SumeragiResponse>(
      fbb.GetBufferPointer(),
      fbb.GetSize()
    );
    /* -------------------------------------------------- */

    return grpc::Status::OK;
  }

  virtual ::grpc::Status Consensus(
    ::grpc::ServerContext *context,
    const flatbuffers::BufferRef<Block> *request,
    flatbuffers::BufferRef<SumeragiResponse> *response) {
    assert(false && "Not implemented yet");
    return grpc::Status::CANCELLED;
  }

private:
  flatbuffers::FlatBufferBuilder fbb;
};

class grpc_test : public testing::Test {
protected:

  void WaitForServerReady() {
    std::unique_lock<std::mutex> lock(wait_for_server);
    while (!server_instance) server_instance_cv.wait(lock);
  }

  // This function is intended for testing client to be able to wait starting server.
  void DelayRunServer() {
    Log("Sleep for testing that server wakes up late...");
    sleep(1);
    RunServer();
  }

  void RunServer() {
    std::string server_address = "0.0.0.0:50051";
    // Callback interface we implemented above.
    SumeragiServiceImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // Start the server. Lock to change the variable we're changing.
    wait_for_server.lock();
    server_instance = builder.BuildAndStart().release();
    wait_for_server.unlock();
    server_instance_cv.notify_one();

    Log("Server listening on " + server_address);
    // This will block the thread and serve requests.
    server_instance->Wait();
  }

  virtual void SetUp() {
    noDebugLog = false;
  }

  //virtual void TearDown() {}

  enum class ServerStartType {
    NoWait, Delay
  };

  void SetUpServer(ServerStartType type) {
    serverThread = std::thread(
      type == ServerStartType::NoWait ?
      &grpc_test::RunServer :
      &grpc_test::DelayRunServer,
      this);
  }

  void TearDownServer() {
    server_instance->Shutdown();
    serverThread.join();
    delete server_instance ;
    server_instance = nullptr;
  }

private:
  std::thread serverThread;
};

class SumeragiClient {
public:
  SumeragiClient(std::shared_ptr<grpc::Channel>&& channel)
    : stub(protocol::Sumeragi::NewStub(std::move(channel)))
  {}

  grpc::Status send(std::vector<uint8_t>& txbuf,
                    flatbuffers::BufferRef<SumeragiResponse>* response)
  {
    grpc::ClientContext context;
    flatbuffers::BufferRef<Transaction> request(
      txbuf.data(), txbuf.size()
    );
    return stub->Torii(&context, request, response);
  }

private:
  std::unique_ptr<protocol::Sumeragi::Stub> stub;
};

TEST_F(grpc_test, Sumeragi) {

  SetUpServer(ServerStartType::Delay);
  WaitForServerReady();

  flatbuffers::FlatBufferBuilder fbb;
  auto act = generator::random_AccountAddAccount(fbb);
  std::vector<flatbuffers::Offset<protocol::ActionWrapper>> actions {
    act
  };
  auto att = generator::random_attachment(fbb);
  auto txbuf = generator::random_tx(fbb, actions, att);

  SumeragiClient client(
    grpc::CreateChannel(
      "localhost:50051",
      grpc::InsecureChannelCredentials()
    )
  );

  flatbuffers::BufferRef<SumeragiResponse> response;
  auto stat = client.send(txbuf, &response);
  ASSERT_TRUE(stat.ok());

  auto reply = response.GetRoot();
  ASSERT_STREQ(reply->message()->c_str(), "OK");
  ASSERT_EQ(reply->code(), protocol::ResponseCode::OK);

  TearDownServer();
}

TEST_F(grpc_test, SecondTry) {

  SetUpServer(ServerStartType::Delay);
  WaitForServerReady();

  flatbuffers::FlatBufferBuilder fbb;
  auto act = generator::random_AccountAddAccount(fbb);
  std::vector<flatbuffers::Offset<protocol::ActionWrapper>> actions {
    act
  };
  auto att = generator::random_attachment(fbb);
  auto txbuf = generator::random_tx(fbb, actions, att);

  SumeragiClient client(
    grpc::CreateChannel(
      "localhost:50051",
      grpc::InsecureChannelCredentials()
    )
  );

  flatbuffers::BufferRef<SumeragiResponse> response;
  auto stat = client.send(txbuf, &response);
  ASSERT_TRUE(stat.ok());

  auto reply = response.GetRoot();
  ASSERT_STREQ(reply->message()->c_str(), "OK");
  ASSERT_EQ(reply->code(), protocol::ResponseCode::OK);

  TearDownServer();
}

TEST_F(grpc_test, StressTestServer) {
  // We can do it many times, but it's too hard for CI.
  for (int i = 0; i < 10 /* 1000 */; i++) {
    std::cout << "Times " << i + 1 << ": ";
    SetUpServer(ServerStartType::NoWait);
    WaitForServerReady();
    TearDownServer();
  }
}

TEST_F(grpc_test, StressTestSendTx) {

  SetUpServer(ServerStartType::NoWait);
  WaitForServerReady();

  noDebugLog = true;

  for (int i = 0; i < 1000; i++) {

    flatbuffers::FlatBufferBuilder fbb;
    auto act = generator::random_AccountAddAccount(fbb);
    std::vector<flatbuffers::Offset<protocol::ActionWrapper>> actions {
      act
    };
    auto att = generator::random_attachment(fbb);
    auto txbuf = generator::random_tx(fbb, actions, att);

    SumeragiClient client(
      grpc::CreateChannel(
        "localhost:50051",
        grpc::InsecureChannelCredentials()
      )
    );

    flatbuffers::BufferRef<SumeragiResponse> response;
    auto stat = client.send(txbuf, &response);
    ASSERT_TRUE(stat.ok());

    auto reply = response.GetRoot();
    ASSERT_STREQ(reply->message()->c_str(), "OK");
    ASSERT_EQ(reply->code(), protocol::ResponseCode::OK);
  }

  TearDownServer();
}
