/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_VARIANT_HPP
#define IROHA_BLOCK_VARIANT_HPP

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
      using VariantType::VariantType;

      interface::types::HeightType height() const override {
        return iroha::visit_in_place(
            *this, [](const auto &any_block) { return any_block->height(); });
      }

      const interface::types::HashType &prevHash() const override {
        return std::move(iroha::visit_in_place(
            *this,
            [](const auto &any_block) { return any_block->prevHash(); }));
      }

      const interface::types::BlobType &blob() const override {
        return std::move(iroha::visit_in_place(
            *this, [](const auto &any_block) { return any_block->blob(); }));
      }

      interface::types::SignatureRangeType signatures() const override {
        return std::move(iroha::visit_in_place(
            *this,
            [](const auto &any_block) { return any_block->signatures(); }));
      }

      bool addSignature(const crypto::Signed &signed_blob,
                        const crypto::PublicKey &public_key) override {
        return std::move(iroha::visit_in_place(
            *this, [&signed_blob, &public_key](const auto &any_block) {
              return any_block->addSignature(signed_blob, public_key);
            }));
      }

      interface::types::TimestampType createdTime() const override {
        return std::move(iroha::visit_in_place(
            *this,
            [](const auto &any_block) { return any_block->createdTime(); }));
      }

      const interface::types::BlobType &payload() const override {
        return std::move(iroha::visit_in_place(
            *this, [](const auto &any_block) { return any_block->payload(); }));
      }

     protected:
      BlockVariant *clone() const override {
        return new BlockVariant(*this);
      }
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_BLOCK_VARIANT_HPP
