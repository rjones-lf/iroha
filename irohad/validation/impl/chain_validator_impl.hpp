/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CHAIN_VALIDATOR_IMPL_HPP
#define IROHA_CHAIN_VALIDATOR_IMPL_HPP

#include "validation/chain_validator.hpp"

#include <memory>

#include "interfaces/common_objects/types.hpp"
#include "logger/logger.hpp"

namespace iroha {

  namespace consensus {
    namespace yac {
      class SupermajorityChecker;
    }  // namespace yac
  }    // namespace consensus

  namespace ametsuchi {
    class PeerQuery;
  }  // namespace ametsuchi

  namespace validation {
    class ChainValidatorImpl : public ChainValidator {
     public:
      ChainValidatorImpl(std::shared_ptr<consensus::yac::SupermajorityChecker>
                             supermajority_checker);

      bool validateChain(
          rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
              blocks,
          ametsuchi::MutableStorage &storage) const override;

     private:
      bool applyBlock(
          const shared_model::interface::Block &block,
          ametsuchi::PeerQuery &queries,
          const shared_model::interface::types::HashType &top_hash) const;

      /**
       * Provide functions to check supermajority
       */
      std::shared_ptr<consensus::yac::SupermajorityChecker>
          supermajority_checker_;

      logger::Logger log_;
    };
  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_CHAIN_VALIDATOR_IMPL_HPP
