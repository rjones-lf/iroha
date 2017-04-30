/*
Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef IROHA_FLATBUFFER_SERVICE_H
#define IROHA_FLATBUFFER_SERVICE_H

#include <utils/expected.hpp>
#include <vector>
#include <functional>
#include <memory>

namespace iroha {
struct Transaction;
struct TransactionWrapper;
struct ConsensusEvent;
struct Peer;
struct PeerAdd;
struct PeerRemove;
struct PeerChangeTrust;
struct PeerSetTrust;
struct PeerSetActive;
struct Signature;
struct Sumeragi;
enum class Command : uint8_t;
}

namespace peer {
struct Node;
}

namespace flatbuffers {
template<class T> class Offset;
class FlatBufferBuilder;

// FIXME: this typedef is dirty and unstable solution. (might be able to be solved by setting dependency for this header)
typedef std::unique_ptr<uint8_t, std::function<void(uint8_t * /* unused */)>> unique_ptr_t;
}

namespace flatbuffer_service {

using ::iroha::Peer;
using ::iroha::PeerAdd;
using ::iroha::PeerRemove;
using ::iroha::PeerChangeTrust;
using ::iroha::PeerSetTrust;
using ::iroha::PeerSetActive;
using ::iroha::Transaction;

Expected<int> hasRequreMember(const iroha::Transaction &tx);

flatbuffers::Offset<void> CreateCommandDirect(
    flatbuffers::FlatBufferBuilder &_fbb, const void *obj,
    iroha::Command type);

Expected<flatbuffers::Offset<::iroha::Transaction>> copyTransaction(
  flatbuffers::FlatBufferBuilder& fbb, const ::iroha::Transaction& fromTx);

Expected<flatbuffers::Offset<::iroha::ConsensusEvent>> copyConsensusEvent(
    flatbuffers::FlatBufferBuilder &fbb, const ::iroha::ConsensusEvent &);

std::vector<uint8_t> CreateAccountBuffer(
  const char* publicKey, const char* alias,
  const std::vector<std::string>& signatories, uint16_t useKeys);

std::vector<uint8_t > GetTxPointer(const iroha::Transaction& tx);


template <typename T>
VoidHandler ensureNotNull(T *value) {
  if (value == nullptr) {
    return makeUnexpected(
        exception::connection::NullptrException(typeid(T).name()));
  }
  return {};
}

std::string toString(const iroha::Transaction &tx);

Expected<flatbuffers::unique_ptr_t> addSignature(const iroha::ConsensusEvent &event,
                                                 const std::string &publicKey,
                                                 const std::string &signature);

Expected<flatbuffers::Offset<::iroha::TransactionWrapper>> toTransactionWrapper(flatbuffers::FlatBufferBuilder&, const ::iroha::Transaction&);
Expected<flatbuffers::unique_ptr_t> toConsensusEvent(const iroha::Transaction &tx);
Expected<flatbuffers::unique_ptr_t> makeCommit(const iroha::ConsensusEvent &event);

namespace peer { // namespace peer

flatbuffers::Offset<PeerAdd> CreateAdd(const ::peer::Node &peer);

flatbuffers::Offset<PeerRemove> CreateRemove(const std::string &pubKey);

flatbuffers::Offset<PeerChangeTrust> CreateChangeTrust(const std::string &pubKey, double &delta);

flatbuffers::Offset<PeerSetTrust> CreateSetTrust(const std::string &pubKey, double &trust);

flatbuffers::Offset<PeerSetActive> CreateSetActive(const std::string &pubKey, bool active);

};

namespace primitives {
  std::vector<uint8_t> CreatePeer(const ::peer::Node &peer);
  std::vector<uint8_t> CreateSignature(const std::string &publicKey,
                                       std::vector<uint8_t> signature,
                                       uint64_t timestamp);
}

namespace transaction { // namespace transaction

const Transaction* CreateTransaction(flatbuffers::FlatBufferBuilder& fbb, iroha::Command cmd_type,
                              flatbuffers::Offset<void> command,
                              std::string creator,
                              std::vector<flatbuffers::Offset<iroha::Signature>> sigs);

}
};
#endif  // IROHA_FLATBUFFER_SERVICE_H
