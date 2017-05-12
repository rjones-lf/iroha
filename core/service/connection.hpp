/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp
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
#ifndef __CONNECTION__
#define __CONNECTION__

#include <utils/expected.hpp>

#include <main_generated.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace iroha {
struct Transaction;
struct ConsensusEvent;
struct Ping;
}

namespace flatbuffers {
// FIXME: this typedef is dirty and unstable solution. (might be able to be
// solved by setting dependency for this header)
typedef std::unique_ptr<uint8_t, std::function<void(uint8_t* /* unused */)>>
    unique_ptr_t;
}

namespace connection {

struct Config {
  std::string name;
  std::string ip_addr;
  std::string port;
};

namespace iroha {
namespace SumeragiImpl {
namespace Verify {

using CallBackFunc = std::function<void(
    const std::string& /* from */, flatbuffers::unique_ptr_t&& /* message */)>;

bool send(const std::string& ip, const ::iroha::ConsensusEvent& msg);
bool sendAll(const ::iroha::ConsensusEvent& msg);
void receive(Verify::CallBackFunc&& callback);

}  // namespace Verify
}  // namespace SumeragiImpl
}  // namespace iroha

namespace iroha {
namespace SumeragiImpl {
namespace Torii {

using CallBackFunc = std::function<void(
    const std::string& /* from */, flatbuffers::unique_ptr_t&& /* message */)>;
void receive(Torii::CallBackFunc&& callback);
/*
namespace HostDiscovery {
    namespace getHostInfo {
        bool receive(const std::function<void(
                const std::string &,
                DiscoverRequest &)> &callback
        );
        Peer send(
                const std::string &ip,
                const DiscoverRequest &message
        );
    }
}
}*/
}
}
}  // namespace iroha::SumeragiImpl::Verify

    namespace iroha {
        namespace AssetRepositoryImpl {
            namespace AccountGetAsset {
                using CallBackFunc = std::function<std::vector<const ::iroha::Asset *>(
                        const std::string & /* from */, flatbuffers::unique_ptr_t && /* message */)>;

                void receive(AccountGetAsset::CallBackFunc &&callback);
            }
        }
    }  // namespace iroha::AssetRepositoryImpl

/************************************************************************************
* Kagami ( means Ping )
************************************************************************************/
namespace memberShipService {
namespace HijiriImpl {
namespace Kagami {
bool send(const std::string& ip, const ::iroha::Ping& ping);
}  // namespace Kagami
}  // namespace HijiriImpl
namespace SumeragiImpl {
namespace Torii {
bool send(const std::string& ip, const ::iroha::Transaction& tx);
}  // namespace Torii
}  // namespace SumeragiImpl
}  // namespace memberShipService

/************************************************************************************
 * Main connection
 ************************************************************************************/
void initialize_peer();
void wait_till_ready();
int run();
void finish();

}  // namespace connection

#endif