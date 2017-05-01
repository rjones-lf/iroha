/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <flatbuffers/flatbuffers.h>
#include <grpc++/grpc++.h>
#include <utils/datetime.hpp>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <endpoint.grpc.fb.h>
#include <main_generated.h>

using grpc::Channel;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ClientContext;
using grpc::Status;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Plz IP " << std::endl;
    std::cout << "Usage: test_sumeragi  ip-address " << std::endl;
    return 1;
  }
  std::cout << "IP:" << argv[1] << std::endl;


  auto channel = grpc::CreateChannel(std::string(argv[1]) + ":50051",
                                     grpc::InsecureChannelCredentials());
  auto stub = iroha::Sumeragi::NewStub(channel);

  grpc::ClientContext context;

  auto publicKey = "SamplePublicKey";
  // Build a request with the name set.
  flatbuffers::FlatBufferBuilder fbb;

  auto account_vec = [&] {
    flatbuffers::FlatBufferBuilder fbbAccount;

    std::unique_ptr<std::vector<flatbuffers::Offset<flatbuffers::String>>>
        signatories(new std::vector<flatbuffers::Offset<flatbuffers::String>>(
            {fbbAccount.CreateString("publicKey1")}));

    auto account = iroha::CreateAccountDirect(fbbAccount, publicKey, "PrevPubKey", "alias",
                                              signatories.get(), 1);
    fbbAccount.Finish(account);

    std::unique_ptr<std::vector<uint8_t>> account_vec(
        new std::vector<uint8_t>());

    auto buf = fbbAccount.GetBufferPointer();

    account_vec->assign(buf, buf + fbbAccount.GetSize());

    return account_vec;
  }();

  auto command = iroha::CreateAccountAddDirect(fbb, account_vec.get());
  /*
      std::unique_ptr<std::vector<flatbuffers::Offset<iroha::Signature>>>
     signature_vec(
          new std::vector<flatbuffers::Offset<iroha::Signature>>()
      );
      signature_vec->emplace_back(iroha::CreateSignatureDirect(fbb,publicKey,
     nullptr,1234567));
  */
  std::vector<uint8_t> signatureBlob{'a', 'b', 'c', 'd'};
  std::vector<uint8_t> hashBlob{'b', 'e', 'e', 'f'};
  std::vector<uint8_t> dataBlob{'d', 'e', 'a', 'd'};

  std::vector<flatbuffers::Offset<iroha::Signature>> signatureOffset_vec{
      iroha::CreateSignatureDirect(fbb, publicKey, &signatureBlob)};
  auto tx_offset = iroha::CreateTransactionDirect(
      fbb, publicKey, iroha::Command::AccountAdd, command.Union(),
      &signatureOffset_vec, &hashBlob, datetime::unixtime(),
      iroha::CreateAttachmentDirect(fbb, "none", &dataBlob));
  fbb.Finish(tx_offset);
  auto tx = flatbuffers::BufferRef<iroha::Transaction>(fbb.GetBufferPointer(),
                                                       fbb.GetSize());

  flatbuffers::BufferRef<iroha::Response> response;
  /*
  {
          message:   string;
          code:      Code;
          signature: Signature;
  }
  */

  // The actual RPC.
  auto status = stub->Torii(&context, tx, &response);
  if (status.ok()) {
    auto msg = response.GetRoot()->message();
    std::cout << "RPC response: " << msg->str() << std::endl;
  } else {
    std::cout << "RPC failed" << std::endl;
  }
  return 0;
}