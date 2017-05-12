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
#include <endpoint_generated.h>
#include <main_generated.h>

using grpc::Channel;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ClientContext;
using grpc::Status;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Plz input target IP " << std::endl;
        std::cout << "Usage: check_ametsuchi publicKey ip-address " << std::endl;
        return 1;
    }
    std::cout << "OwnPublicKey:" << argv[1] << std::endl;
    std::cout << "TargetIP:"     << argv[2] << std::endl;

    auto channel = grpc::CreateChannel(std::string(argv[2]) + ":50051",
         grpc::InsecureChannelCredentials()
    );
    auto stub = iroha::AssetRepository::NewStub(channel);

    grpc::ClientContext context;

    auto publicKey = argv[1];
    // Build a request with the name set.
    flatbuffers::FlatBufferBuilder fbb;

    std::string ln = "iroha-develop", dn = "default", an = "iroha";

    auto query_offset = iroha::CreateAssetQueryDirect(
        fbb, publicKey, ln.c_str(), dn.c_str(), an.c_str()
    );
    fbb.Finish(query_offset);
    auto query = flatbuffers::BufferRef<iroha::AssetQuery>(
            fbb.GetBufferPointer(),fbb.GetSize());

    flatbuffers::BufferRef<iroha::AssetResponse> response;

    // The actual RPC.
    auto status = stub->AccountGetAsset(&context, query, &response);
    if (status.ok()) {
        auto msg = response.GetRoot()->message();
        auto assets = response.GetRoot()->assets();
        std::cout << "RPC response: " << msg->str() << std::endl;
        for(const auto& a: *assets){
            if(reinterpret_cast<const iroha::Asset*>(a)->asset_type() == iroha::AnyAsset::Currency){
                std::cout << "ledger:" << reinterpret_cast<const iroha::Asset*>(a)->
                        asset_as_Currency()->ledger_name()->str()
                    << " domain:" << reinterpret_cast<const iroha::Asset*>(a)->asset_as_Currency()->domain_name()->str()
                    << " asset:" << reinterpret_cast<const iroha::Asset*>(a)->asset_as_Currency()->currency_name()->str()
                    << "   amonut:" << reinterpret_cast<const iroha::Asset*>(a)->asset_as_Currency()->amount()->str() << std::endl;
            }
        }
    } else {
        std::cout << "RPC failed" << std::endl;
    }
    return 0;
}