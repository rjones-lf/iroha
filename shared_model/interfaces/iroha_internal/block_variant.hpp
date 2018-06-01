/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_VARIANT_HPP
#define IROHA_BLOCK_VARIANT_HPP

#include "common/visitor.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/empty_block.hpp"

namespace shared_model {
  namespace interface {

    class BlockVariant
        : public boost::variant<
              std::shared_ptr<shared_model::interface::Block>,
              std::shared_ptr<shared_model::interface::EmptyBlock>>,
          protected AbstractBlock {
     private:
      using VariantType =
          boost::variant<std::shared_ptr<shared_model::interface::Block>,
                         std::shared_ptr<shared_model::interface::EmptyBlock>>;

     public:
      using AbstractBlock::hash;
      using VariantType::VariantType;

      interface::types::HeightType height() const override;

      const interface::types::HashType &prevHash() const override;

      const interface::types::BlobType &blob() const override;

      interface::types::SignatureRangeType signatures() const override;

      bool addSignature(const crypto::Signed &signed_blob,
                        const crypto::PublicKey &public_key) override;

      interface::types::TimestampType createdTime() const override;

      const interface::types::BlobType &payload() const override;

      bool operator==(const BlockVariant &rhs) const;

     protected:
      BlockVariant *clone() const override;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_BLOCK_VARIANT_HPP
