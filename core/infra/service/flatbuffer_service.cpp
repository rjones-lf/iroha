
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

#include <service/flatbuffer_service.h>
#include <time.h>
#include <crypto/base64.hpp>
#include <crypto/hash.hpp>
#include <crypto/signature.hpp>
#include <utils/datetime.hpp>
#include <utils/expected.hpp>
#include <utils/logger.hpp>

#include <flatbuffers/flatbuffers.h>
#include <main_generated.h>
#include <iostream>
#include <map>
#include <infra/config/peer_service_with_json.hpp>
#include <membership_service/peer_service.hpp>
#include <memory>
#include <string>
#include <commands_generated.h>

namespace flatbuffer_service {

  flatbuffers::Offset<void> CreateCommandFromTx(
    flatbuffers::FlatBufferBuilder& fbb, const Transaction& tx) {

    using iroha::Command;

    switch (tx.command_type()) {
      case Command::NONE: {
        assert(false && "Command::NONE");
      }
      case Command::Add: {
        auto p = tx.command_as_Add();
        std::vector<uint8_t> asset(p->asset()->begin(), p->asset()->end());
        return ::iroha::CreateAddDirect(
          fbb, p->accPubKey()->c_str(), &asset).Union();
      }
      case Command::Subtract: {
        auto p = tx.command_as_Subtract();
        std::vector<uint8_t> asset(p->asset()->begin(), p->asset()->end());
        return ::iroha::CreateSubtractDirect(
          fbb, p->accPubKey()->c_str(), &asset).Union();
      }
      case Command::Transfer: {
        auto p = tx.command_as_Transfer();
        std::vector<uint8_t> asset(p->asset()->begin(), p->asset()->end());
        return ::iroha::CreateTransferDirect(
          fbb, &asset, p->sender()->c_str(), p->receiver()->c_str()).Union();
      }
      case Command::AssetCreate: {
        auto p = tx.command_as_AssetCreate();
        return ::iroha::CreateAssetCreateDirect(
          fbb, p->asset_name()->c_str(), p->domain_name()->c_str(),
          p->ledger_name()->c_str()).Union();
      }
      case Command::AssetRemove: {
        auto p = tx.command_as_AssetRemove();
        return ::iroha::CreateAssetRemoveDirect(
          fbb, p->asset_name()->c_str(), p->domain_name()->c_str(),
          p->ledger_name()->c_str()).Union();
      }
      case Command::PeerAdd: {
        auto p = tx.command_as_PeerAdd();
        std::vector<uint8_t> peer(p->peer()->begin(), p->peer()->end());
        return ::iroha::CreatePeerAddDirect(fbb, &peer).Union();
      }
      case Command::PeerRemove: {
        auto p = tx.command_as_PeerRemove();
        return ::iroha::CreatePeerRemoveDirect(
          fbb, p->peerPubKey()->c_str()).Union();
      }
      case Command::PeerSetActive: {
        auto p = tx.command_as_PeerSetActive();
        return ::iroha::CreatePeerSetActiveDirect(
          fbb, p->peerPubKey()->c_str(), p->active()).Union();
      }
      case Command::PeerSetTrust: {
        auto p = tx.command_as_PeerSetTrust();
        return ::iroha::CreatePeerSetTrustDirect(
          fbb, p->peerPubKey()->c_str(), p->trust()).Union();
      }
      case Command::PeerChangeTrust: {
        auto p = tx.command_as_PeerChangeTrust();
        return ::iroha::CreatePeerChangeTrustDirect(
          fbb, p->peerPubKey()->c_str(), p->delta()).Union();
      }
      case Command::AccountAdd: {
        auto p = tx.command_as_AccountAdd();
        std::vector<uint8_t> account(p->account()->begin(), p->account()->end());
        return ::iroha::CreateAccountAddDirect(fbb, &account).Union();
      }
      case Command::AccountRemove: {
        auto p = tx.command_as_AccountRemove();
        return ::iroha::CreateAccountRemoveDirect(fbb, p->pubkey()->c_str()).Union();
      }

      // WILL CHECK
      case Command::AccountAddSignatory: {
        auto p = tx.command_as_AccountAddSignatory();
        std::vector<flatbuffers::Offset<flatbuffers::String>>
          signatory(p->signatory()->begin(), p->signatory()->end());
        return ::iroha::CreateAccountAddSignatoryDirect(
          fbb, p->account()->c_str(), &signatory).Union();
      }
      case Command::AccountRemoveSignatory: {
        auto p = tx.command_as_AccountRemoveSignatory();
        std::vector<flatbuffers::Offset<flatbuffers::String>>
          signatory(p->signatory()->begin(), p->signatory()->end());
        return ::iroha::CreateAccountRemoveSignatoryDirect(
          fbb, p->account()->c_str(), &signatory).Union();
      }
      case Command::AccountSetUseKeys: {
        auto p = tx.command_as_AccountSetUseKeys();
        std::vector<flatbuffers::Offset<flatbuffers::String>>
          accounts(p->accounts()->begin(), p->accounts()->end());
        return ::iroha::CreateAccountSetUseKeysDirect(fbb, &accounts, p->useKeys()).Union();
      }
      case Command::AccountMigrate: {
        auto p = tx.command_as_AccountMigrate();
        std::vector<uint8_t> account(p->account()->begin(), p->account()->end());
        return ::iroha::CreateAccountMigrateDirect(fbb, &account, p->prevPubKey()->c_str()).Union();
      }
      case Command::ChaincodeAdd: {
        throw exception::NotImplementedException("Command", __FILE__);
      }
      case Command::ChaincodeRemove: {
        throw exception::NotImplementedException("Command", __FILE__);
      }
      case Command::ChaincodeExecute: {
        throw exception::NotImplementedException("Command", __FILE__);
      }
      case Command::PermissionRemove: {
        /*
        auto p = tx.command_as_PermissionRemove();
        ::iroha::CreateP
        return ::iroha::CreatePermissionRemoveDirect(fbb, p->targetAccount()->c_str(), p->permission_type(), );
         */
        throw exception::NotImplementedException("Command", __FILE__);
      }
      case Command::PermissionAdd: {
        /*
        auto p = tx.command_as_AccountMigrate();
        std::vector<uint8_t> account(p->account()->begin(), p->account()->end());
        return ::iroha::CreateAccountMigrateDirect(fbb, &account, p->prevPubKey()->c_str()).Union();
         */
        throw exception::NotImplementedException("Command", __FILE__);
      }
      default: return flatbuffers::Offset<void>();
    }
  }

  std::string toStringOf(iroha::Command cmd_type, const iroha::Transaction& tx) {
    using iroha::Command;
    switch (cmd_type) {
      case Command::PeerAdd: {
        auto p = tx.command_as_PeerAdd();
        auto root = p->peer_nested_root();
        std::string ret;
        ret += root->ledger_name()->str();
        ret += root->publicKey()->str();
        ret += root->ip()->str();
        ret += std::to_string(root->trust());
        ret += std::to_string(root->active());
        ret += std::to_string(root->join_ledger());
        return ret;
      }
      case Command::PeerRemove: {
        auto p = tx.command_as_PeerRemove();
        std::string ret;
        ret += p->peerPubKey()->str();
        return ret;
      }
      case Command::PeerSetActive: {
        auto p = tx.command_as_PeerSetActive();
        std::string ret;
        ret += p->peerPubKey()->str();
        ret += std::to_string(p->active());
        return ret;
      }
      case Command::PeerSetTrust: {
        auto p = tx.command_as_PeerSetTrust();
        std::string ret;
        ret += p->peerPubKey()->str();
        ret += std::to_string(p->trust());
        return ret;
      }
      case Command::PeerChangeTrust: {
        auto p = tx.command_as_PeerChangeTrust();
        std::string ret;
        ret += p->peerPubKey()->str();
        ret += std::to_string(p->delta());
        return ret;
      }
      default:
        throw exception::NotImplementedException("peer toStringOf", __FILE__);
    }
  }

  std::string toStringAttachmentOf(const Transaction& tx) {
    auto ret = tx.attachment()->mime()->str();
    ret += std::string(tx.attachment()->data()->begin(),
                       tx.attachment()->data()->end());
    return ret;
  }

  // ToDo: We should use this for only debug. dump();
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
          res += "[broken]\n";
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
        res += "        amount:" + ast->amount()->str() + "\n";
        res += "        precision:" + std::to_string(ast->precision()) + "\n";
        res += "    ]\n";
        return res;
      };

    command_to_strings[iroha::Command::AssetCreate] =
      [&](const void* command) -> std::string {
        const iroha::AssetCreate* cmd =
          static_cast<const iroha::AssetCreate*>(command);

        std::string res = "AssetCreate[\n";
        // res += "    creatorPubKey:" + cmd->creatorPubKey()->str() + ",\n"; No
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
    command_to_strings[iroha::Command::Add] =
      [&](const void* command) -> std::string {
        const iroha::Add* cmd = static_cast<const iroha::Add*>(command);

        std::string res = "Add[\n";
        res += "    accPubkey:" + cmd->accPubKey()->str() + ",\n";
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
        res += "    peer:join_ledger:" +
               std::to_string(cmd->peer_nested_root()->join_ledger()) + "\n";
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
        res += "    account:pubKey:" + std::string(cmd->pubkey()->c_str());
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
     * copyTransactionsOf(event)
     * - copies transactions from event and write data to given FlatBufferBuilder.
     */
    Expected<std::vector<flatbuffers::Offset<::iroha::TransactionWrapper>>>
    copyTxWrappersOfEvent(flatbuffers::FlatBufferBuilder& fbb,
                          const ::iroha::ConsensusEvent& event) {
      std::vector<flatbuffers::Offset<::iroha::TransactionWrapper>> txwrappers;

      for (auto&& tx : *event.transactions()) {
        std::vector<uint8_t> nested(tx->tx()->begin(), tx->tx()->end());
        txwrappers.push_back(::iroha::CreateTransactionWrapperDirect(fbb, &nested));
      }

      return txwrappers;
    }
  }  // namespace detail

  /**
   * copyTransaction(fromTx)
   * - copies transaction and write data to given FlatBufferBuilder.
   */
  Expected<flatbuffers::Offset<::iroha::Transaction>> copyTransaction(
    flatbuffers::FlatBufferBuilder& fbb, const ::iroha::Transaction& fromTx) {
    auto tx_signatures = detail::copySignaturesOfTx(fbb, fromTx);
    if (!tx_signatures) {
      return makeUnexpected(tx_signatures.excptr());
    }

    auto hash = detail::copyHashOfTx(fromTx);
    if (!hash) {
      return makeUnexpected(hash.excptr());
    }

    auto attachment = detail::copyAttachmentOfTx(fbb, fromTx);
    if (!attachment) {
      return makeUnexpected(attachment.excptr());
    }

    const auto pubkey = fromTx.creatorPubKey()->c_str();
    const auto cmdtype = fromTx.command_type();
    const auto cmd = flatbuffer_service::CreateCommandFromTx(fbb, fromTx);

    return ::iroha::CreateTransactionDirect(fbb, pubkey, cmdtype, cmd,
                                            &tx_signatures.value(), &hash.value(),
                                            fromTx.timestamp(),
                                            attachment.value());
  }

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
    auto txwrappers = detail::copyTxWrappersOfEvent(fbb, event);
    if (!txwrappers) {
      return makeUnexpected(txwrappers.excptr());
    }
    return ::iroha::CreateConsensusEventDirect(fbb, &peerSignatures.value(),
                                               &txwrappers.value(), event.code());
  }

  /**
   * toTxWrapper(fbb, tx)
   * - wrap transaction to TransactionWrapper
   */
  Expected<flatbuffers::Offset<::iroha::TransactionWrapper>> toTxWrapper(
    flatbuffers::FlatBufferBuilder& fbb, const ::iroha::Transaction& tx) {
    flatbuffers::FlatBufferBuilder xbb;
    auto txOffset = copyTransaction(xbb, tx);
    if (!txOffset) {
      return makeUnexpected(txOffset.excptr());
    }
    xbb.Finish(txOffset.value());
    auto ptr = xbb.GetBufferPointer();
    std::vector<uint8_t> nested(ptr, ptr + xbb.GetSize());

    return ::iroha::CreateTransactionWrapperDirect(fbb, &nested);
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

    auto txwOffset = toTxWrapper(fbb, fromTx);

    std::vector<flatbuffers::Offset<::iroha::TransactionWrapper>> txs;
    txs.push_back(*txwOffset);

    auto consensusEventOffset = ::iroha::CreateConsensusEventDirect(
      fbb, &peerSignatureOffsets, &txs, ::iroha::Code::UNDECIDED);
    fbb.Finish(consensusEventOffset);
    return fbb.ReleaseBufferPointer();
  }

  Expected<flatbuffers::unique_ptr_t> addSignature(
    const iroha::ConsensusEvent& event, const std::string& publicKey,
    const std::string& signature) {
    flatbuffers::FlatBufferBuilder fbb(16);

    std::vector<flatbuffers::Offset<iroha::Signature>> peerSignatures;

    // ToDo: #(tx) = 1
    for (const auto& aPeerSig : *event.peerSignatures()) {
      std::vector<uint8_t> aPeerSigBlob(aPeerSig->signature()->begin(),
                                        aPeerSig->signature()->end());
      peerSignatures.push_back(::iroha::CreateSignatureDirect(
        fbb, aPeerSig->publicKey()->c_str(), &aPeerSigBlob,
        aPeerSig->timestamp()));
    }

    std::vector<uint8_t> aNewPeerSigBlob;
    for (auto& c : signature) {
      aNewPeerSigBlob.push_back(c);
    }

    // ToDo: Migrate flatbuffer_service::primitives::CreateSignature()
    peerSignatures.push_back(
      ::iroha::CreateSignatureDirect(fbb, publicKey.c_str(),
                                     &aNewPeerSigBlob, datetime::unixtime()));

    // ToDo: #(tx) = 1
    auto tx = std::vector<uint8_t>(event.transactions()->Get(0)->tx()->begin(),
                                   event.transactions()->Get(0)->tx()->end());

    std::vector<flatbuffers::Offset<iroha::TransactionWrapper>> txwrappers;
    txwrappers.push_back(
      ::iroha::CreateTransactionWrapperDirect(fbb, &tx));

    auto consensusEventOffset = ::iroha::CreateConsensusEventDirect(
      fbb, &peerSignatures, &txwrappers, event.code());

    fbb.Finish(consensusEventOffset);
    return fbb.ReleaseBufferPointer();
  }

  Expected<flatbuffers::unique_ptr_t> makeCommit(
    const iroha::ConsensusEvent& event) {
    flatbuffers::FlatBufferBuilder fbb(16);

    std::vector<flatbuffers::Offset<iroha::Signature>> peerSignatures;

    for (const auto& aPeerSig : *event.peerSignatures()) {
      std::vector<uint8_t> aPeerSigBlob(aPeerSig->signature()->begin(),
                                        aPeerSig->signature()->end());
      peerSignatures.push_back(::iroha::CreateSignatureDirect(
        fbb, aPeerSig->publicKey()->c_str(), &aPeerSigBlob,
        aPeerSig->timestamp()));
    }

    auto txwrappers = detail::copyTxWrappersOfEvent(fbb, event);
    if (!txwrappers) {
      return makeUnexpected(txwrappers.excptr());
    }

    auto consensusEventOffset = ::iroha::CreateConsensusEventDirect(
      fbb, &peerSignatures, &txwrappers.value(),
      iroha::Code::COMMIT);

    fbb.Finish(consensusEventOffset);
    return fbb.ReleaseBufferPointer();
  }

  namespace peer {  // namespace peer

    flatbuffers::Offset<PeerAdd> CreateAdd(flatbuffers::FlatBufferBuilder &fbb, const ::peer::Node &peer) {
      return iroha::CreatePeerAdd(fbb, fbb.CreateVector(primitives::CreatePeer(peer)));
    }

    flatbuffers::Offset<PeerRemove> CreateRemove(flatbuffers::FlatBufferBuilder &fbb, const std::string &pubKey) {
      return iroha::CreatePeerRemove(fbb, fbb.CreateString(pubKey));
    }

    flatbuffers::Offset<PeerChangeTrust> CreateChangeTrust(
      flatbuffers::FlatBufferBuilder &fbb,
      const std::string &pubKey, double delta) {
      return iroha::CreatePeerChangeTrust(fbb, fbb.CreateString(pubKey), delta);
    }

    flatbuffers::Offset<PeerSetTrust> CreateSetTrust(
      flatbuffers::FlatBufferBuilder &fbb,
      const std::string &pubKey, double trust) {
      return iroha::CreatePeerSetTrust(fbb, fbb.CreateString(pubKey), trust);
    }

    flatbuffers::Offset<PeerSetActive> CreateSetActive(
      flatbuffers::FlatBufferBuilder &fbb,
      const std::string &pubKey,
      bool active) {
      return iroha::CreatePeerSetActive(fbb, fbb.CreateString(pubKey), active);
    }

  };  // namespace peer

  namespace primitives {  // namespace primitives

    std::vector<uint8_t> CreatePeer(const ::peer::Node& peer) {
      flatbuffers::FlatBufferBuilder fbb;
      auto peer_cp = iroha::CreatePeer(
        fbb, fbb.CreateString(peer.ledger_name), fbb.CreateString(peer.publicKey),
        fbb.CreateString(peer.ip), peer.trust, peer.active, peer.join_ledger
      );
      fbb.Finish(peer_cp);

      uint8_t* ptr = fbb.GetBufferPointer();
      return {ptr, ptr + fbb.GetSize()};
    }

    flatbuffers::Offset<::iroha::Signature> CreateSignature(
      flatbuffers::FlatBufferBuilder &fbb, const std::string &hash, uint64_t timestamp) {
      // In oreder to use variable hash and create signature with timestamp,
      // we need hashed string and timestamp in arguments.
      const auto signature = signature::sign(
        hash,
        config::PeerServiceConfig::getInstance().getMyPublicKey(),
        config::PeerServiceConfig::getInstance().getMyPrivateKey());
      const std::vector<uint8_t> sigblob(signature.begin(), signature.end());
      return ::iroha::CreateSignatureDirect(
        fbb, config::PeerServiceConfig::getInstance().getMyPublicKey().c_str(),
        &sigblob, timestamp);
    }

  }  // namespace primitives

  namespace account {

    // Note: This function is used mainly for debug because Sumeragi doesn't create Account.
    std::vector<uint8_t> CreateAccount(
      const std::string& publicKey, const std::string& alias,
      const std::string& prevPubKey, const std::vector<std::string>& signatories,
      uint16_t useKeys) {
      flatbuffers::FlatBufferBuilder fbb;

      std::vector<flatbuffers::Offset<flatbuffers::String>> signatoryOffsets;
      for (const auto& e : signatories) {
        signatoryOffsets.push_back(fbb.CreateString(e));
      }

      auto accountOffset =
        ::iroha::CreateAccountDirect(fbb, publicKey.c_str(), prevPubKey.c_str(),
                                     alias.c_str(), &signatoryOffsets, 1);
      fbb.Finish(accountOffset);

      auto buf = fbb.GetBufferPointer();
      return {buf, buf + fbb.GetSize()};
    }

  }  // namespace account

  namespace asset {

    // Note: This function is used mainly for debug because Sumeragi doesn't create Currency.
    std::vector<uint8_t> CreateCurrency(
      const std::string& currencyName, const std::string& domainName,
      const std::string& ledgerName, const std::string& description,
      const std::string& amount, uint8_t precision
    ) {
      flatbuffers::FlatBufferBuilder fbb;
      auto currency = iroha::CreateCurrencyDirect(fbb, currencyName.c_str(), domainName.c_str(),
                                                  ledgerName.c_str(), description.c_str(),
                                                  amount.c_str(), precision);
      auto asset = iroha::CreateAsset(fbb, ::iroha::AnyAsset::Currency, currency.Union());
      fbb.Finish(asset);
      auto buf = fbb.GetBufferPointer();
      return {buf, buf + fbb.GetSize()};
    }

  }  // namespace asset

  namespace transaction {  // namespace transaction

    Expected<std::vector<uint8_t>> GetTxPointer(const iroha::Transaction &tx){
      flatbuffers::FlatBufferBuilder xbb;
      auto txOffset = copyTransaction(xbb, tx);
      if (!txOffset) {
        return makeUnexpected(txOffset.excptr());
      }
      xbb.Finish(txOffset.value());
      auto ptr = xbb.GetBufferPointer();
      std::vector<uint8_t> nested(ptr, ptr + xbb.GetSize());
      return nested;
    }

    std::vector<uint8_t> CreateTransaction(
      flatbuffers::FlatBufferBuilder& fbb,
      const std::string& creatorPubKey,
      iroha::Command cmd_type,
      const flatbuffers::Offset<void>& command
    ) {
      std::vector<uint8_t> dummy = {'d','u','m','m','y'};
      auto attachment = ::iroha::CreateAttachmentDirect(fbb, "dummy", &dummy);
      return CreateTransaction(
        fbb, creatorPubKey, cmd_type, command, attachment);
    }

    /**
     *  CreateTransaction()
     *  Notice: This function call fbb.Finish()
     */
    std::vector<uint8_t> CreateTransaction(
      flatbuffers::FlatBufferBuilder& fbb,
      const std::string& creatorPubKey,
      iroha::Command cmd_type,
      const flatbuffers::Offset<void>& command,
      flatbuffers::Offset<iroha::Attachment> attachment
    ) {
      const auto timestamp = datetime::unixtime();
      /*
       * sha256(creatorPubKey + command_type + timestamp + attachment)
       * Future work: not command_type but command
       */
      std::string hashable;
      auto appendStr = [&](const std::string& s) {
        if (s.empty()) return;
        for (const auto& e: s) {
          hashable.push_back((char)e);
        }
      };

      auto appendVec = [&](const std::vector<uint8_t>& v) {
        if (v.empty()) return;
        for (const auto& e: v) {
          hashable.push_back((char)e);
        }
      };

      auto atc = flatbuffers::GetTemporaryPointer(fbb, attachment);
      std::string attachstr = atc->mime()->str() +
        std::string(atc->data()->begin(), atc->data()->end());

      appendStr(creatorPubKey);
      appendStr(::iroha::EnumNameCommand(cmd_type));
      appendStr(std::to_string(timestamp));
      appendStr(attachstr);

      const auto hash = hash::sha3_256_hex(hashable);

      std::vector<flatbuffers::Offset<::iroha::Signature>> signatures{
        flatbuffer_service::primitives::CreateSignature(
          fbb, hash, timestamp)};

      std::vector<uint8_t> hsvec(hash.begin(), hash.end());

      auto txOffset = ::iroha::CreateTransactionDirect(
        fbb, creatorPubKey.c_str(), cmd_type, command,
        &signatures, &hsvec, timestamp, attachment);

      fbb.Finish(txOffset);

      auto bufptr = fbb.GetBufferPointer();
      return {bufptr, bufptr + fbb.GetSize()};
    }

  };  // namespace transaction

}  // namespace flatbuffer_service