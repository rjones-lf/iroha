/**
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
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

#include <main_generated.h>
#include <service/flatbuffer_service.h>
#include <utils/datetime.hpp>
#include <utils/expected.hpp>
#include <utils/logger.hpp>
#include <crypto/signature.hpp>
#include <crypto/base64.hpp>
#include <crypto/hash.hpp>
#include <time.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace flatbuffer_service {

/**
 * CreateCommandDirect
 */
flatbuffers::Offset<void> CreateCommandDirect(
    flatbuffers::FlatBufferBuilder& _fbb, const void* obj,
    ::iroha::Command /* Command */ type) {
  switch (type) {
      case ::iroha::Command::NONE: {
      logger::error("flatbuffer service") << "Command_NONE";
      exit(1);
    }
    case ::iroha::Command::AssetCreate: {
      auto ptr = reinterpret_cast<const ::iroha::AssetCreate*>(obj);
      return ::iroha::CreateAssetCreateDirect(
                 _fbb, ptr->asset_name()->c_str(), ptr->domain_name()->c_str(),
                 ptr->ledger_name()->c_str()
      ).Union();
    }
  case ::iroha::Command::AssetAdd: {
      auto ptr = reinterpret_cast<const ::iroha::AssetAdd*>(obj);
      auto asset =
          std::vector<uint8_t>(ptr->asset()->begin(), ptr->asset()->end());
      return ::iroha::CreateAssetAddDirect(_fbb, ptr->accPubKey()->c_str(),
                                           &asset)
          .Union();
    }
      case ::iroha::Command::AssetRemove: {
      auto ptr = reinterpret_cast<const ::iroha::AssetRemove*>(obj);
      auto asset =
          std::vector<uint8_t>(ptr->asset()->begin(), ptr->asset()->end());
      return ::iroha::CreateAssetRemoveDirect(_fbb, ptr->accPubKey()->c_str(),
                                              &asset)
          .Union();
    }
      case ::iroha::Command::AssetTransfer: {
      auto ptr = reinterpret_cast<const ::iroha::AssetTransfer*>(obj);
      auto asset =
          std::vector<uint8_t>(ptr->asset()->begin(), ptr->asset()->end());
      return ::iroha::CreateAssetTransferDirect(
                 _fbb, &asset, ptr->sender()->c_str(), ptr->receiver()->c_str())
          .Union();
    }
      case ::iroha::Command::PeerAdd: {
      auto ptr = reinterpret_cast<const ::iroha::PeerAdd*>(obj);
      auto peer =
          std::vector<uint8_t>(ptr->peer()->begin(), ptr->peer()->end());
      return ::iroha::CreatePeerAddDirect(_fbb, &peer).Union();
    }
      case ::iroha::Command::PeerRemove: {
      auto ptr = reinterpret_cast<const ::iroha::PeerRemove*>(obj);
      return ::iroha::CreatePeerRemoveDirect(_fbb, ptr->peerPubKey()->c_str()).Union();
    }
      case ::iroha::Command::PeerSetActive: {
      auto ptr = reinterpret_cast<const ::iroha::PeerSetActive*>(obj);
      return ::iroha::CreatePeerSetActiveDirect(
                 _fbb, ptr->peerPubKey()->c_str(), ptr->active())
          .Union();
    }
      case ::iroha::Command::PeerSetTrust: {
      auto ptr = reinterpret_cast<const ::iroha::PeerSetTrust*>(obj);
      return ::iroha::CreatePeerSetTrustDirect(_fbb, ptr->peerPubKey()->c_str(),
                                               ptr->trust())
          .Union();
    }
      case ::iroha::Command::PeerChangeTrust: {
      auto ptr = reinterpret_cast<const ::iroha::PeerChangeTrust*>(obj);
      return ::iroha::CreatePeerChangeTrustDirect(
                 _fbb, ptr->peerPubKey()->c_str(), ptr->delta())
          .Union();
    }
      case ::iroha::Command::AccountAdd: {
      auto ptr = reinterpret_cast<const ::iroha::AccountAdd*>(obj);
      auto account =
          std::vector<uint8_t>(ptr->account()->begin(), ptr->account()->end());
      return ::iroha::CreateAccountAddDirect(_fbb, &account).Union();
    }
      case ::iroha::Command::AccountRemove: {
      auto ptr = reinterpret_cast<const ::iroha::AccountRemove*>(obj);
      return ::iroha::CreateAccountRemoveDirect(_fbb, ptr->pubkey()->c_str()).Union();
    }
      case ::iroha::Command::AccountAddSignatory: {
      auto ptr = reinterpret_cast<const ::iroha::AccountAddSignatory*>(obj);
      auto signatory = std::vector<flatbuffers::Offset<flatbuffers::String>>(
          ptr->signatory()->begin(), ptr->signatory()->end());
      return ::iroha::CreateAccountAddSignatoryDirect(
                 _fbb, ptr->account()->c_str(), &signatory)
          .Union();
    }
      case ::iroha::Command::AccountRemoveSignatory: {
      auto ptr = reinterpret_cast<const ::iroha::AccountRemoveSignatory*>(obj);
      auto signatory = std::vector<flatbuffers::Offset<flatbuffers::String>>(
          ptr->signatory()->begin(), ptr->signatory()->end());
      return ::iroha::CreateAccountRemoveSignatoryDirect(
                 _fbb, ptr->account()->c_str(), &signatory)
          .Union();
    }
      case ::iroha::Command::AccountSetUseKeys: {
      auto ptr = reinterpret_cast<const ::iroha::AccountSetUseKeys*>(obj);
      auto accounts = std::vector<flatbuffers::Offset<flatbuffers::String>>(
          ptr->accounts()->begin(), ptr->accounts()->end());
      return ::iroha::CreateAccountSetUseKeysDirect(_fbb, &accounts,
                                                    ptr->useKeys())
          .Union();
    }
      case ::iroha::Command::ChaincodeAdd: {
      auto ptr = reinterpret_cast<const ::iroha::ChaincodeAdd*>(obj);
      auto code =
          std::vector<uint8_t>(ptr->code()->begin(), ptr->code()->end());
      return ::iroha::CreateChaincodeAddDirect(_fbb, &code).Union();
    }
      case ::iroha::Command::ChaincodeRemove: {
      auto ptr = reinterpret_cast<const ::iroha::ChaincodeRemove*>(obj);
      auto code =
          std::vector<uint8_t>(ptr->code()->begin(), ptr->code()->end());
      return ::iroha::CreateChaincodeRemoveDirect(_fbb, &code).Union();
    }
      case ::iroha::Command::ChaincodeExecute: {
      auto ptr = reinterpret_cast<const ::iroha::ChaincodeExecute*>(obj);
      return ::iroha::CreateChaincodeExecuteDirect(
                 _fbb, ptr->code_name()->c_str(), ptr->domain_name()->c_str(),
                 ptr->ledger_name()->c_str())
          .Union();
    }
    default: {
      throw exception::NotImplementedException("No match Command type",
                                               __FILE__);
    }
  }
}

std::vector<uint8_t> CreateAccountBuffer(
    const char* publicKey, const char* alias,
    const std::vector<std::string>& signatories, uint16_t useKeys) {
  if (&signatories != nullptr) {
    flatbuffers::FlatBufferBuilder fbbAccount;

    std::vector<flatbuffers::Offset<flatbuffers::String>> signatoryOffsets;
    for (const auto& e : signatories) {
      signatoryOffsets.push_back(fbbAccount.CreateString(e));
    }

    auto accountOffset = ::iroha::CreateAccountDirect(
        fbbAccount, publicKey, alias, &signatoryOffsets, 1);
    fbbAccount.Finish(accountOffset);

    auto buf = fbbAccount.GetBufferPointer();
    std::vector<uint8_t> buffer;
    buffer.assign(buf, buf + fbbAccount.GetSize());
    return buffer;
  } else {
    flatbuffers::FlatBufferBuilder fbbAccount;
    auto accountOffset =
        ::iroha::CreateAccountDirect(fbbAccount, publicKey, alias, nullptr, 1);
    fbbAccount.Finish(accountOffset);

    auto buf = fbbAccount.GetBufferPointer();
    return std::vector<uint8_t>(buf, buf + fbbAccount.GetSize());
  }
}

/**
 * toString
 * - it returns string dump of arguments' tx like DebugString in protocol
 * buffer. ToDo If transaction scheme is changed, We changes this code.
 */
std::string toString(const iroha::Transaction& tx) {
  std::string res = "";
  if (tx.creatorPubKey() != nullptr) {
    res += "creatorPubKey:" + tx.creatorPubKey()->str() + ",\n";
  }
  if (tx.signatures() != nullptr) {
    res += "signatures:[\n";
    for (const auto& s : *tx.signatures()) {
      if (s->publicKey() != nullptr || s->signature() != nullptr) {
        res += "  [\n    publicKey:" + s->publicKey()->str() + ",\n";
        res += "    signature:" +
               std::string(s->signature()->begin(), s->signature()->end()) +
               ",\n";
        res += "    timestamp:" + std::to_string(s->timestamp()) + "\n  ]\n";
      } else {
        res += "[brolen]\n";
      }
    }
    res += "]\n";
  }
  if (tx.attachment() != nullptr) {
    assert(tx.attachment()->mime() != nullptr);
    assert(tx.attachment()->data() != nullptr);

    res += "attachment:[\n";
    res += " mime:" +
           std::string(tx.attachment()->mime()->begin(),
                       tx.attachment()->mime()->end()) +
           ",\n";
    res += " data:" +
           std::string(tx.attachment()->data()->begin(),
                       tx.attachment()->data()->end()) +
           ",\n";
    res += "]\n";
  }

  std::map<iroha::Command, std::function<std::string(const void*)>>
      command_to_strings;
  std::map<iroha::AnyAsset, std::function<std::string(const void*)>>
      any_asset_to_strings;

  any_asset_to_strings[iroha::AnyAsset::ComplexAsset] =
      [&](const void* asset) -> std::string {
    const iroha::ComplexAsset* ast =
        static_cast<const iroha::ComplexAsset*>(asset);

    std::string res = " ComplexAsset[\n";
    res += "        asset_name:" + ast->asset_name()->str() + ",\n";
    res += "        domain_name:" + ast->domain_name()->str() + ",\n";
    res += "        ledger_name:" + ast->ledger_name()->str() + ",\n";
    res += "        description:" + ast->description()->str() + "\n";
    res += "        asset:logic:WIP\n";
    res += "    ]\n";
    return res;
  };
  any_asset_to_strings[iroha::AnyAsset::Currency] =
      [&](const void* asset) -> std::string {
    const iroha::Currency* ast = static_cast<const iroha::Currency*>(asset);

    std::string res = " Currency[\n";
    res += "        currency_name:" + ast->currency_name()->str() + ",\n";
    res += "        domain_name:" + ast->domain_name()->str() + ",\n";
    res += "        ledger_name:" + ast->ledger_name()->str() + ",\n";
    res += "        description:" + ast->description()->str() + "\n";
    res += "        amount:" + std::to_string(ast->amount()) + "\n";
    res += "        precision:" + std::to_string(ast->precision()) + "\n";
    res += "    ]\n";
    return res;
  };

  command_to_strings[iroha::Command::AssetCreate] =
      [&](const void* command) -> std::string {
    const iroha::AssetCreate* cmd =
        static_cast<const iroha::AssetCreate*>(command);

    std::string res = "AssetCreate[\n";
    res += "    ledger_name:" + cmd->ledger_name()->str() + ",\n";
    res += "    domain_name:" + cmd->domain_name()->str() + ",\n";
    res += "    asset_name:" + cmd->asset_name()->str() + "\n";
    res += "]\n";
    return res;
  };

  command_to_strings[iroha::Command::AssetCreate] =
      [&](const void* command) -> std::string {
    const iroha::AssetCreate* cmd =
        static_cast<const iroha::AssetCreate*>(command);

    std::string res = "AssetCreate[\n";
    res += "    ledger_name:" + cmd->ledger_name()->str() + ",\n";
    res += "    domain_name:" + cmd->domain_name()->str() + ",\n";
    res += "    asset_name:" + cmd->asset_name()->str() + "\n";
    res += "]\n";
    return res;
  };
  command_to_strings[iroha::Command::AssetAdd] =
      [&](const void* command) -> std::string {
    const iroha::AssetAdd* cmd = static_cast<const iroha::AssetAdd*>(command);

    std::string res = "AssetAdd[\n";
    res += "    accPubKey:" + cmd->accPubKey()->str() + ",\n";
    res += "    asset:" +
           any_asset_to_strings[cmd->asset_nested_root()->asset_type()](
               cmd->asset_nested_root()->asset());
    res += "]\n";
    return res;
  };
  command_to_strings[iroha::Command::AssetRemove] =
      [&](const void* command) -> std::string {
    const iroha::AssetRemove* cmd =
        static_cast<const iroha::AssetRemove*>(command);

    std::string res = "AssetRemove[\n";
    res += "    accPubKey:" + cmd->accPubKey()->str() + ",\n";
    res += "    asset:" +
           any_asset_to_strings[cmd->asset_nested_root()->asset_type()](
               cmd->asset_nested_root()->asset());
    res += "]\n";
    return res;
  };
  command_to_strings[iroha::Command::AssetTransfer] =
      [&](const void* command) -> std::string {
    const iroha::AssetTransfer* cmd =
        static_cast<const iroha::AssetTransfer*>(command);

    std::string res = "AssetTransfer[\n";
    res += "    sender:" + cmd->sender()->str() + ",\n";
    res += "    receiver:" + cmd->receiver()->str() + ",\n";
    res += "    asset:" +
           any_asset_to_strings[cmd->asset_nested_root()->asset_type()](
               cmd->asset_nested_root()->asset());
    res += "]\n";
    return res;
  };

  command_to_strings[iroha::Command::PeerAdd] =
      [&](const void* command) -> std::string {
    const iroha::PeerAdd* cmd = static_cast<const iroha::PeerAdd*>(command);

    std::string res = "PeerAdd[\n";
    res += "    peer:publicKey:" + cmd->peer_nested_root()->publicKey()->str() +
           ",\n";
    res += "    peer:ip:" + cmd->peer_nested_root()->ip()->str() + ",\n";
    res += "    peer:active:" +
           std::to_string(cmd->peer_nested_root()->active()) + ",\n";
    res += "    peer:join_network:" +
           std::to_string(cmd->peer_nested_root()->join_network()) + ",\n";
    res += "    peer:join_validation:" +
           std::to_string(cmd->peer_nested_root()->join_validation()) + "\n";
    res += "]\n";
    return res;
  };
  command_to_strings[iroha::Command::PeerRemove] =
      [&](const void* command) -> std::string {
    const iroha::PeerRemove* cmd =
        static_cast<const iroha::PeerRemove*>(command);

    std::string res = "PeerRemove[\n";
    res += "    peer:publicKey:" + cmd->peerPubKey()->str();
    res += "\n]\n";
    return res;
  };
  command_to_strings[iroha::Command::PeerSetActive] =
      [&](const void* command) -> std::string {
    const iroha::PeerSetActive* cmd =
        static_cast<const iroha::PeerSetActive*>(command);

    std::string res = "PeerSetActive[\n";
    res += "    peer:peerPubKey:" + cmd->peerPubKey()->str() + ",\n";
    res += "]\n";
    return res;
  };
  command_to_strings[iroha::Command::PeerSetTrust] =
      [&](const void* command) -> std::string {
    const iroha::PeerSetTrust* cmd =
        static_cast<const iroha::PeerSetTrust*>(command);

    std::string res = "PeerSetTrust[\n";
    res += "    peerPubKey:" + cmd->peerPubKey()->str() + ",\n";
    res += "    trust:" + std::to_string(cmd->trust()) + ",\n";
    res += "]\n";
    return res;
  };
  command_to_strings[iroha::Command::PeerChangeTrust] =
      [&](const void* command) -> std::string {
    const iroha::PeerChangeTrust* cmd =
        static_cast<const iroha::PeerChangeTrust*>(command);

    std::string res = "PeerSetTrust[\n";
    res += "    peerPubKey:" + cmd->peerPubKey()->str() + ",\n";
    res += "    delta:" + std::to_string(cmd->delta()) + "\n";
    res += "]\n";
    return res;
  };

  command_to_strings[iroha::Command::AccountAdd] =
      [&](const void* command) -> std::string {
    const iroha::AccountAdd* cmd =
        static_cast<const iroha::AccountAdd*>(command);
    std::string res = "AccountAdd[\n";
    if (cmd->account_nested_root() != nullptr) {
      if (cmd->account_nested_root()->alias() != 0) {
        res += "    account:alias:" +
               cmd->account_nested_root()->alias()->str() + ",\n";
      }
      if (cmd->account_nested_root()->pubKey() != nullptr) {
        res += "    account:pubKey:" +
               cmd->account_nested_root()->pubKey()->str() + ",\n";
      }
      if (cmd->account_nested_root()->signatories() != nullptr) {
        for (const auto& s : *cmd->account_nested_root()->signatories()) {
          res += "        signature[" + s->str() + "]\n";
        }
      }
    }
    res += "]\n";
    return res;
  };
  command_to_strings[iroha::Command::AccountRemove] =
      [&](const void* command) -> std::string {
    const iroha::AccountRemove* cmd =
        static_cast<const iroha::AccountRemove*>(command);

    std::string res = "AccountRemove[\n";
    res +=     cmd->pubkey()->c_str();
    res += "]\n";
    return res;
  };
  command_to_strings[iroha::Command::AccountAddSignatory] =
      [&](const void* command) -> std::string {
    const iroha::AccountAddSignatory* cmd =
        static_cast<const iroha::AccountAddSignatory*>(command);

    std::string res = "AccountAddSignatory[\n";
    res += "    account:" + cmd->account()->str() + ",\n";
    for (const auto& s : *cmd->signatory()) {
      res += "        signature[" + s->str() + "]\n";
    }
    res += "]\n";
    return res;
  };
  command_to_strings[iroha::Command::AccountRemoveSignatory] =
      [&](const void* command) -> std::string {
    const iroha::AccountRemoveSignatory* cmd =
        static_cast<const iroha::AccountRemoveSignatory*>(command);

    std::string res = "AccountRemoveSignatory[\n";
    res += "    account:" + cmd->account()->str() + ",\n";
    for (const auto& s : *cmd->signatory()) {
      res += "        signature[" + s->str() + "]\n";
    }
    res += "]\n";
    return res;
  };
  command_to_strings[iroha::Command::AccountSetUseKeys] =
      [&](const void* command) -> std::string {
    const iroha::AccountSetUseKeys* cmd =
        static_cast<const iroha::AccountSetUseKeys*>(command);

    std::string res = "AccountSetUseKeys[\n";
    for (const auto& a : *cmd->accounts()) {
      res += "        account[" + a->str() + "]\n";
    }
    res += "    account:useKeys:" + std::to_string(cmd->useKeys()) + "\n";
    res += "]\n";
    return res;
  };

  command_to_strings[iroha::Command::ChaincodeAdd] =
      [&](const void* command) -> std::string {
    const iroha::ChaincodeAdd* cmd =
        static_cast<const iroha::ChaincodeAdd*>(command);

    std::string res = "ChaincodeAdd[\n";
    res += "]\n";
    return res;
  };
  command_to_strings[iroha::Command::ChaincodeRemove] =
      [&](const void* command) -> std::string {
    const iroha::ChaincodeRemove* cmd =
        static_cast<const iroha::ChaincodeRemove*>(command);

    std::string res = "ChaincodeRemove[\n";
    res += "]\n";
  };
  command_to_strings[iroha::Command::ChaincodeExecute] =
      [&](const void* command) -> std::string {
    const iroha::ChaincodeExecute* cmd =
        static_cast<const iroha::ChaincodeExecute*>(command);

    std::string res = "ChaincodeExecute[\n";
    res += "]\n";
  };
  res += command_to_strings[tx.command_type()](tx.command());
  return res;
}

namespace detail {
/**
 * copyPeerSignatures
 * - copies peer signatures of consensus event and write data to given
 *   FlatBufferBuilder
 */
Expected<std::vector<flatbuffers::Offset<::iroha::Signature>>>
copyPeerSignaturesOf(flatbuffers::FlatBufferBuilder& fbb,
                     const ::iroha::ConsensusEvent& event) {
  std::vector<flatbuffers::Offset<::iroha::Signature>> peerSignatures;

  for (const auto& aPeerSig : *event.peerSignatures()) {
    // TODO: Check signature is broken.
    VoidHandler handler;
    handler = ensureNotNull(aPeerSig);
    if (!handler) {
      logger::error("Connection with grpc") << "Peer signature is null";
      return makeUnexpected(handler.excptr());
    }

    handler = ensureNotNull(aPeerSig->signature());
    if (!handler) {
      logger::error("Connection with grpc") << "Peer signature is null";
      return makeUnexpected(handler.excptr());
    }

    std::vector<uint8_t> aPeerSigBlob(aPeerSig->signature()->begin(),
                                      aPeerSig->signature()->end());
    peerSignatures.push_back(
        ::iroha::CreateSignatureDirect(fbb, aPeerSig->publicKey()->c_str(),
                                       &aPeerSigBlob, aPeerSig->timestamp()));
  }

  return peerSignatures;
}

Expected<std::vector<flatbuffers::Offset<::iroha::Signature>>>
copySignaturesOfTx(flatbuffers::FlatBufferBuilder& fbb,
                   const ::iroha::Transaction& fromTx) {
  std::vector<flatbuffers::Offset<::iroha::Signature>> tx_signatures;

  auto handler = ensureNotNull(fromTx.signatures());
  if (!handler) {
    logger::error("Connection with grpc") << "Transaction signature is null";
    return makeUnexpected(handler.excptr());
  }

  for (auto&& txSig : *fromTx.signatures()) {
    auto handler = ensureNotNull(txSig->signature());
    if (!handler) {
      logger::error("Connection with grpc") << "Transaction signature is null";
      return makeUnexpected(handler.excptr());
    }

    std::vector<uint8_t> _data(txSig->signature()->begin(),
                               txSig->signature()->end());

    tx_signatures.emplace_back(iroha::CreateSignatureDirect(
        fbb, txSig->publicKey()->c_str(), &_data, txSig->timestamp()));
  }

  return tx_signatures;
}

Expected<std::vector<uint8_t>> copyHashOfTx(
    const ::iroha::Transaction& fromTx) {
  auto handler = ensureNotNull(fromTx.hash());
  if (!handler) {
    logger::error("Connection with grpc") << "Transaction hash is null";
    return makeUnexpected(handler.excptr());
  }
  return std::vector<uint8_t>(fromTx.hash()->begin(), fromTx.hash()->end());
}

Expected<flatbuffers::Offset<::iroha::Attachment>> copyAttachmentOfTx(
    flatbuffers::FlatBufferBuilder& fbb, const ::iroha::Transaction& fromTx) {
  VoidHandler handler;
  handler = ensureNotNull(fromTx.attachment());
  if (!handler) {
    logger::error("Connection with grpc") << "Transaction attachment is null";
    return makeUnexpected(handler.excptr());
  }

  handler = ensureNotNull(fromTx.attachment()->data());
  if (!handler) {
    logger::error("Connection with grpc")
        << "Transaction attachment's data is null";
    return makeUnexpected(handler.excptr());
  }

  std::vector<uint8_t> data(fromTx.attachment()->data()->begin(),
                            fromTx.attachment()->data()->end());
  return iroha::CreateAttachmentDirect(
      fbb, fromTx.attachment()->mime()->c_str(), &data);
}

/**
 * copyTransaction(fromTx)
 * - copies transaction and write data to given FlatBufferBuilder.
 */
Expected<flatbuffers::Offset<::iroha::Transaction>> copyTransaction(
    flatbuffers::FlatBufferBuilder& fbb, const ::iroha::Transaction& fromTx) {
  auto tx_signatures = copySignaturesOfTx(fbb, fromTx);
  if (!tx_signatures) {
    return makeUnexpected(tx_signatures.excptr());
  }

  auto hash = copyHashOfTx(fromTx);
  if (!hash) {
    return makeUnexpected(hash.excptr());
  }

  auto attachment = copyAttachmentOfTx(fbb, fromTx);
  if (!attachment) {
    return makeUnexpected(attachment.excptr());
  }

  // ToDo: Currently, #(transaction) is one.
  const auto pubkey = fromTx.creatorPubKey()->c_str();
  const auto cmdtype = fromTx.command_type();
  const auto cmd = flatbuffer_service::CreateCommandDirect(
      fbb, fromTx.command(), fromTx.command_type());

  return ::iroha::CreateTransactionDirect(fbb, pubkey, cmdtype, cmd,
                                          &tx_signatures.value(), &hash.value(),
                                          attachment.value());
}

/**
 * copyTransactionsOf(event)
 * - copies transactions from event and write data to given FlatBufferBuilder.
 */
Expected<std::vector<flatbuffers::Offset<::iroha::TransactionWrapper>>>
copyTransactionsWrapperOf(flatbuffers::FlatBufferBuilder& fbb,
                   const ::iroha::ConsensusEvent& event) {
  std::vector<flatbuffers::Offset<::iroha::TransactionWrapper>> transactions;
  for (auto&& tx : *event.transactions()) {
    flatbuffers::FlatBufferBuilder fbbTx;
    auto txOffset = copyTransaction( fbbTx, *tx->tx_nested_root());
    if (!txOffset) {
      return makeUnexpected(txOffset.excptr());
    }
    auto buf = fbbTx.GetBufferPointer();
    std::vector<uint8_t> buffer;
    buffer.assign(buf, buf + fbbTx.GetSize());

    transactions.push_back(::iroha::CreateTransactionWrapper(
        fbb,
        &buffer
    ));
  }

  return transactions;
}
}  // namespace detail

/**
 * copyConsensusEvent(event)
 * - copies consensus event and write data to given FlatBufferBuilder.
 *
 * Returns: Expected<Offset<ConsensusEvent>>
 */
Expected<flatbuffers::Offset<::iroha::ConsensusEvent>> copyConsensusEvent(
    flatbuffers::FlatBufferBuilder& fbb, const iroha::ConsensusEvent& event) {
  auto peerSignatures = detail::copyPeerSignaturesOf(fbb, event);
  if (!peerSignatures) {
    return makeUnexpected(peerSignatures.excptr());
  }
  auto transactions = detail::copyTransactionsWrapperOf(fbb, event);
  if (!transactions) {
    return makeUnexpected(transactions.excptr());
  }

  return ::iroha::CreateConsensusEventDirect(
      fbb, &peerSignatures.value(), &transactions.value(), event.code()
  );
}

/**
 * toConsensusEvent
 * - Encapsulate the transaction given from Torii(client) in a consensus event.
 * Argument fromTx will be deeply copied and create new consensus event that has
 * the copied transaction. - After creating new consensus event,
 * addSignature() is called from sumeragi. So, the new event has empty
 * peerSignatures.
 *
 * Returns: Expected<unique_ptr_t>
 */
Expected<flatbuffers::unique_ptr_t> toConsensusEvent(
    const iroha::Transaction& fromTx) {
  flatbuffers::FlatBufferBuilder fbb(16);

  std::vector<flatbuffers::Offset<::iroha::Signature>>
      peerSignatureOffsets;  // Empty.

  // TODO: multiple transaction
  flatbuffers::FlatBufferBuilder fbbTx;
  auto txOffset = detail::copyTransaction(fbbTx, fromTx);
  if (!txOffset) {
    return makeUnexpected(txOffset.excptr());
  }
  auto buf = fbbTx.GetBufferPointer();
  std::vector<uint8_t> buffer;
  buffer.assign(buf, buf + fbbTx.GetSize());

  std::vector<flatbuffers::Offset<::iroha::TransactionWrapper>> transactions;
  transactions.push_back(::iroha::CreateTransactionWrapperDirect(
      fbb, &buffer
  ));

  auto consensusEventOffset = ::iroha::CreateConsensusEventDirect(
      fbb, &peerSignatureOffsets, &transactions, ::iroha::Code::UNDECIDED);

  fbb.Finish(consensusEventOffset);
  return fbb.ReleaseBufferPointer();
}

flatbuffers::unique_ptr_t addSignature(const iroha::ConsensusEvent& event,
                                       const std::string& publicKey,
                                       const std::string& signature) {
  flatbuffers::FlatBufferBuilder fbbConsensusEvent(16);

  std::vector<flatbuffers::Offset<iroha::Signature>> peerSignatures;
  std::vector<flatbuffers::Offset<iroha::Signature>> signatures;

  // Tempolary implementation: Currently, #(tx) is one.
  auto tx = event.transactions()->Get(0)->tx_nested_root();
  const auto& aSignature = tx->signatures()->Get(0);
  const auto& aPeerSignatures = event.peerSignatures();

  for (const auto& aPeerSig : *event.peerSignatures()) {
    std::vector<uint8_t> aPeerSigBlob(aPeerSig->signature()->begin(),
                                      aPeerSig->signature()->end());
    peerSignatures.push_back(::iroha::CreateSignatureDirect(
        fbbConsensusEvent, aPeerSig->publicKey()->c_str(), &aPeerSigBlob,
        aPeerSig->timestamp()));
  }

  std::vector<uint8_t> aNewPeerSigBlob;  // ToDo: Does it need to sign()?
  for (auto& c : signature) {
    aNewPeerSigBlob.push_back(c);
  }
  peerSignatures.push_back(::iroha::CreateSignatureDirect(
      fbbConsensusEvent, aSignature->publicKey()->c_str(), &aNewPeerSigBlob,
      datetime::unixtime()));


  std::vector<uint8_t> signatureBlob(aSignature->signature()->begin(),
                                     aSignature->signature()->end());

  signatures.push_back(::iroha::CreateSignatureDirect(
      fbbConsensusEvent, aSignature->publicKey()->c_str(), &signatureBlob,
      aSignature->timestamp()));

  std::vector<uint8_t> hashes;
  if (tx->hash() != nullptr) {
    hashes.assign(tx->hash()->begin(), tx->hash()->end());
  }

  flatbuffers::Offset<::iroha::Attachment> attachmentOffset;
  std::vector<uint8_t> data;
  if (tx->attachment() != nullptr && tx->attachment()->data() != nullptr &&
      tx->attachment()->mime() != nullptr) {
    data.assign(tx->attachment()->data()->begin(),
                tx->attachment()->data()->end());

    attachmentOffset = ::iroha::CreateAttachmentDirect(
        fbbConsensusEvent, tx->attachment()->mime()->c_str(), &data);
  }

  std::vector<flatbuffers::Offset<::iroha::TransactionWrapper>> transactions;

    flatbuffers::FlatBufferBuilder fbbTx;
    auto txOffset = ::iroha::CreateTransactionDirect(
        fbbTx, tx->creatorPubKey()->c_str(), tx->command_type(),
        flatbuffer_service::CreateCommandDirect(fbbConsensusEvent, tx->command(),
                                                tx->command_type()),
        &signatures, &hashes, attachmentOffset);

    auto buf = fbbTx.GetBufferPointer();
    std::vector<uint8_t> buffer;
    buffer.assign(buf, buf + fbbTx.GetSize());

    transactions.push_back(::iroha::CreateTransactionWrapper(
            fbbConsensusEvent,
            &buffer
    ));

  auto consensusEventOffset = ::iroha::CreateConsensusEventDirect(
      fbbConsensusEvent, &peerSignatures, &transactions, event.code());

  fbbConsensusEvent.Finish(consensusEventOffset);
  return fbbConsensusEvent.ReleaseBufferPointer();
}

flatbuffers::unique_ptr_t makeCommit(const iroha::ConsensusEvent& event) {
  flatbuffers::FlatBufferBuilder fbbConsensusEvent(16);

  // At first, peerSignatures is empty. (Is this right?)
  std::vector<flatbuffers::Offset<iroha::Signature>> peerSignatures;
  std::vector<flatbuffers::Offset<iroha::Signature>> signatures;

  // Tempolary implementation: Currently, #(tx) is one.
  auto tx = event.transactions()->Get(0)->tx_nested_root();
  const auto& aSignature = tx->signatures()->Get(0);
  const auto& aPeerSignatures = event.peerSignatures();

  for (const auto& aPeerSig : *event.peerSignatures()) {
    std::vector<uint8_t> aPeerSigBlob(aPeerSig->signature()->begin(),
                                      aPeerSig->signature()->end());
    peerSignatures.push_back(::iroha::CreateSignatureDirect(
        fbbConsensusEvent, aPeerSig->publicKey()->c_str(), &aPeerSigBlob,
        1234567));
  }

  std::vector<uint8_t> signatureBlob(aSignature->signature()->begin(),
                                     aSignature->signature()->end());

  signatures.push_back(::iroha::CreateSignatureDirect(
      fbbConsensusEvent, aSignature->publicKey()->c_str(), &signatureBlob,
      1234567));

  std::vector<uint8_t> hashes;
  if (tx->hash() != nullptr) {
    hashes.assign(tx->hash()->begin(), tx->hash()->end());
  }

  flatbuffers::Offset<::iroha::Attachment> attachmentOffset;
  std::vector<uint8_t> data;
  if (tx->attachment() != nullptr && tx->attachment()->data() != nullptr &&
      tx->attachment()->mime() != nullptr) {
    data.assign(tx->attachment()->data()->begin(),
                tx->attachment()->data()->end());

    attachmentOffset = ::iroha::CreateAttachmentDirect(
        fbbConsensusEvent, tx->attachment()->mime()->c_str(), &data);
  }

  std::vector<flatbuffers::Offset<::iroha::TransactionWrapper>> transactions;

  flatbuffers::FlatBufferBuilder fbbTx;
  auto txOffset = ::iroha::CreateTransactionDirect(
          fbbTx, tx->creatorPubKey()->c_str(), tx->command_type(),
          flatbuffer_service::CreateCommandDirect(fbbConsensusEvent, tx->command(),
                                                  tx->command_type()),
          &signatures, &hashes, attachmentOffset);

  auto buf = fbbTx.GetBufferPointer();
  std::vector<uint8_t> buffer;
  buffer.assign(buf, buf + fbbTx.GetSize());

  transactions.push_back(::iroha::CreateTransactionWrapper(
          fbbConsensusEvent,
          &buffer
  ));

  auto consensusEventOffset = ::iroha::CreateConsensusEventDirect(
      fbbConsensusEvent, &peerSignatures, &transactions, iroha::Code::COMMIT);

  fbbConsensusEvent.Finish(consensusEventOffset);
  return fbbConsensusEvent.ReleaseBufferPointer();
}


namespace peer { // namespace peer

flatbuffers::Offset<PeerAdd> CreateAdd(const ::peer::Node &peer){
  flatbuffers::FlatBufferBuilder fbb;
  return iroha::CreatePeerAdd( fbb, fbb.CreateVector( primitives::CreatePeer(peer) ) );
}
flatbuffers::Offset<PeerRemove> CreateRemove(const std::string& pubKey){
  flatbuffers::FlatBufferBuilder fbb;
  return iroha::CreatePeerRemove( fbb, fbb.CreateString(pubKey) );

}
flatbuffers::Offset<PeerChangeTrust> CreateChangeTrust(const std::string& pubKey,double& delta){
  flatbuffers::FlatBufferBuilder fbb;
  return iroha::CreatePeerChangeTrust( fbb, fbb.CreateString( pubKey ), delta );
}
flatbuffers::Offset<PeerSetTrust> CreateSetTrust(const std::string& pubKey,double& trust){
  flatbuffers::FlatBufferBuilder fbb;
  return iroha::CreatePeerSetTrust( fbb, fbb.CreateString(pubKey), trust );

}
flatbuffers::Offset<PeerSetActive> CreateSetActive(const std::string& pubKey,bool active){
  flatbuffers::FlatBufferBuilder fbb;
  return iroha::CreatePeerSetActive( fbb, fbb.CreateString(pubKey), active);
}

};

namespace primitives { // namespace primitives

std::vector<uint8_t> CreatePeer(const ::peer::Node &peer) {
  flatbuffers::FlatBufferBuilder fbb;
  auto peer_cp = iroha::CreatePeer( fbb, fbb.CreateString(peer.publicKey),
                                    fbb.CreateString(peer.ip), peer.trust, peer.active, peer.join_network, peer.join_validation );
  fbb.Finish( peer_cp );

  uint8_t* ptr = fbb.GetBufferPointer();
  return {ptr, ptr + fbb.GetSize()};
}

}

namespace transaction { // namespace transaction

const Transaction& CreateTransaction(flatbuffers::FlatBufferBuilder& fbb, iroha::Command cmd_type,
                              flatbuffers::Offset<void> command,
                              std::string creator,
                              std::vector<flatbuffers::Offset<iroha::Signature>> sigs) {
  flatbuffers::FlatBufferBuilder xbb;
  auto tx_mt = iroha::CreateTransaction( xbb, xbb.CreateString(creator), cmd_type, command, xbb.CreateVector(sigs) );
  xbb.Finish(tx_mt);
  auto hash = hash::sha3_256_hex( toString( *flatbuffers::GetRoot<Transaction>( xbb.GetBufferPointer() ) ) );

  auto tx = iroha::CreateTransaction( fbb, fbb.CreateString(creator),
                                   cmd_type, command, fbb.CreateVector(sigs),
                                   fbb.CreateVector( static_cast<std::vector<uint8_t>>( base64::decode(hash) ) ) );
  fbb.Finish( tx );
  return *flatbuffers::GetRoot<Transaction>(fbb.GetBufferPointer());
}

};



}  // namespace flatbuffer_service
