/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/optional.hpp>

#include "backend/protobuf/transaction.hpp"
#include "backend/protobuf/util.hpp"
#include "bindings/client_api.hpp"
#include "common/bind.hpp"
#include "cryptography/crypto_provider/crypto_signer.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "validators/default_validator.hpp"

namespace shared_model {
  namespace bindings {

    std::string convert(const Blob &blob) {
      return std::string(blob.begin(), blob.end());
    }

    template <typename Proto>
    boost::optional<Proto> get(const std::string &blob) {
      Proto proto;
      if (proto.ParseFromString(blob)) {
        return proto;
      }
      return {};
    }

    using namespace iroha;
    void validateTransaction(const Blob &b) {
      auto blob = convert(b);
      auto s = get<iroha::protocol::Transaction>(blob) | [](auto tx) {
        static validation::DefaultSignedTransactionValidator val;
        return boost::make_optional(
            val.validate(proto::Transaction(tx)).reason());
      };
      if (s) {
        auto &r = s.value();
        if (r == "") {
          return;
        }
        throw std::invalid_argument(r);
      }
      throw std::invalid_argument("unknown object");
    }

    void validateQuery(const Blob &b) {
      auto blob = convert(b);
      auto s = get<iroha::protocol::Query>(blob) | [](auto qry) {
        static validation::DefaultSignedQueryValidator val;
        return boost::make_optional(val.validate(proto::Query(qry)).reason());
      };
      if (s) {
        auto &r = s.value();
        if (r == "") {
          return;
        }
        throw std::invalid_argument(r);
      }
      throw std::invalid_argument("unknown object");
    }

    Blob signTransaction(const Blob &b, const crypto::Keypair &key) {
      auto blob = convert(b);
      auto s = get<iroha::protocol::Transaction>(blob) | [&key](auto tx) {
        auto signature =
            crypto::CryptoSigner<>::sign(proto::makeBlob(tx.payload()), key);

        auto sig = tx.add_signatures();
        sig->set_signature(crypto::toBinaryString(signature));
        sig->set_public_key(crypto::toBinaryString(key.publicKey()));
        return boost::make_optional(tx);
      };
      if (s) {
        return proto::makeBlob(s.value()).blob();
      }
      throw std::invalid_argument("unknown object");
    }

    Blob signQuery(const Blob &b, const crypto::Keypair &key) {
      auto blob = convert(b);
      auto s = get<iroha::protocol::Query>(blob) | [&key](auto qry) {
        auto signature =
            crypto::CryptoSigner<>::sign(proto::makeBlob(qry.payload()), key);

        auto sig = qry.mutable_signature();
        sig->set_signature(crypto::toBinaryString(signature));
        sig->set_public_key(crypto::toBinaryString(key.publicKey()));
        return boost::make_optional(qry);
      };
      if (s) {
        return proto::makeBlob(s.value()).blob();
      }
      throw std::invalid_argument("unknown object");
    }

    Blob hashTransaction(const Blob &b) {
      auto blob = convert(b);
      auto s = get<iroha::protocol::Transaction>(blob) | [](auto tx) {
        auto pl = proto::makeBlob(tx.payload());
        return boost::make_optional(
            sha3_256(pl.blob().data(), pl.blob().size()));
      };
      if (s) {
        return Blob(s->begin(), s->end());
      }
      throw std::invalid_argument("unknown object");
    }

    Blob hashQuery(const Blob &b) {
      auto blob = convert(b);
      auto s = get<iroha::protocol::Query>(blob) | [](auto qry) {
        auto pl = proto::makeBlob(qry.payload());
        return boost::make_optional(
            sha3_256(pl.blob().data(), pl.blob().size()));
      };
      if (s) {
        return Blob(s->begin(), s->end());
      }
      throw std::invalid_argument("unknown object");
    }

    interface::types::HashType utxReducedHash(
        const shared_model::proto::UnsignedWrapper<
            shared_model::proto::Transaction> &utx) {
      return utx.reducedHash();
    }
  }  // namespace bindings
}  // namespace shared_model
