/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_YAC_PB_CONVERTERS_HPP
#define IROHA_YAC_PB_CONVERTERS_HPP

#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"
#include "common/byteutils.hpp"
#include "consensus/yac/outcome_messages.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "logger/logger.hpp"
#include "validators/field_validator.hpp"
#include "yac.pb.h"

namespace iroha {
  namespace consensus {
    namespace yac {
      class PbConverters {
       private:
        static inline proto::Vote serializeRoundAndHashes(
            const VoteMessage &vote) {
          proto::Vote pb_vote;

          auto hash = pb_vote.mutable_hash();
          auto hash_round = hash->mutable_vote_round();
          hash_round->set_block_round(vote.hash.vote_round.block_round);
          hash_round->set_reject_round(vote.hash.vote_round.reject_round);
          auto hash_vote_hashes = hash->mutable_vote_hashes();
          hash_vote_hashes->set_proposal(vote.hash.vote_hashes.proposal_hash);
          hash_vote_hashes->set_block(vote.hash.vote_hashes.block_hash);

          return pb_vote;
        }

        static inline VoteMessage deserealizeRoundAndHashes(
            const proto::Vote &pb_vote) {
          VoteMessage vote;

          vote.hash.vote_round =
              Round{pb_vote.hash().vote_round().block_round(),
                    pb_vote.hash().vote_round().reject_round()};
          vote.hash.vote_hashes =
              YacHash::VoteHashes{pb_vote.hash().vote_hashes().proposal(),
                                  pb_vote.hash().vote_hashes().block()};

          return vote;
        }

       public:
        static proto::Vote serializeVotePayload(const VoteMessage &vote) {
          auto pb_vote = serializeRoundAndHashes(vote);

          if (vote.hash.block_signature) {
            auto block_signature =
                pb_vote.mutable_hash()->mutable_block_signature();
            block_signature->set_signature(shared_model::crypto::toBinaryString(
                vote.hash.block_signature->signedData()));
            block_signature->set_pubkey(shared_model::crypto::toBinaryString(
                vote.hash.block_signature->publicKey()));
          }

          return pb_vote;
        }

        static proto::Vote serializeVote(const VoteMessage &vote) {
          auto pb_vote = serializeRoundAndHashes(vote);

          if (vote.hash.block_signature) {
            auto block_signature =
                pb_vote.mutable_hash()->mutable_block_signature();
            block_signature->set_signature(shared_model::crypto::toBinaryString(
                vote.hash.block_signature->signedData()));
            block_signature->set_pubkey(shared_model::crypto::toBinaryString(
                vote.hash.block_signature->publicKey()));
          }

          auto signature = pb_vote.mutable_signature();
          const auto &sig = *vote.signature;
          signature->set_signature(
              shared_model::crypto::toBinaryString(sig.signedData()));
          signature->set_pubkey(
              shared_model::crypto::toBinaryString(sig.publicKey()));

          return pb_vote;
        }

        static boost::optional<VoteMessage> deserializeVote(
            const proto::Vote &pb_vote, logger::LoggerPtr log) {
          static shared_model::proto::ProtoCommonObjectsFactory<
              shared_model::validation::FieldValidator>
              factory_;

          auto vote = deserealizeRoundAndHashes(pb_vote);

          auto deserialize =
              [&](auto &pubkey, auto &signature, auto &val, const auto &msg) {
                factory_
                    .createSignature(shared_model::crypto::PublicKey(pubkey),
                                     shared_model::crypto::Signed(signature))
                    .match(
                        [&](iroha::expected::Value<
                            std::unique_ptr<shared_model::interface::Signature>>
                                &sig) { val = std::move(sig.value); },
                        [&](iroha::expected::Error<std::string> &reason) {
                          log->error(msg, reason.error);
                        });
              };

          if (pb_vote.hash().has_block_signature()) {
            deserialize(pb_vote.hash().block_signature().pubkey(),
                        pb_vote.hash().block_signature().signature(),
                        vote.hash.block_signature,
                        "Cannot build vote hash block signature: {}");
          }

          deserialize(pb_vote.signature().pubkey(),
                      pb_vote.signature().signature(),
                      vote.signature,
                      "Cannot build vote signature: {}");

          return vote;
        }
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_PB_CONVERTERS_HPP
