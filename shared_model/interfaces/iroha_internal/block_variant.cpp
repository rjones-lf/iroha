/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/block_variant.hpp"

namespace shared_model {
  namespace interface {

    template <typename Func, typename... Args>
    static auto invoke(const BlockVariant *var, Func &&f, Args &&... args) {
      return iroha::visit_in_place(
          *var, [&](const auto &any_block) -> decltype(auto) {
            return ((*any_block.*f)(std::forward<Args>(args)...));
          });
    }

    interface::types::HeightType BlockVariant::height() const {
      return invoke(this, &AbstractBlock::height);
    }

    const interface::types::HashType &BlockVariant::prevHash() const {
      return std::move(invoke(this, &AbstractBlock::prevHash));
    }

    const interface::types::BlobType &BlockVariant::blob() const {
      return std::move(invoke(this, &AbstractBlock::blob));
    }

    interface::types::SignatureRangeType BlockVariant::signatures() const {
      return std::move(invoke(this, &AbstractBlock::signatures));
    }

    bool BlockVariant::addSignature(const crypto::Signed &signed_blob,
                                    const crypto::PublicKey &public_key) {
      return invoke(
          this, &AbstractBlock::addSignature, signed_blob, public_key);
    }

    interface::types::TimestampType BlockVariant::createdTime() const {
      return std::move(invoke(this, &AbstractBlock::createdTime));
    }

    const interface::types::BlobType &BlockVariant::payload() const {
      return std::move(invoke(this, &AbstractBlock::payload));
    }

    bool BlockVariant::operator==(const BlockVariant &rhs) const {
      return AbstractBlock::operator==(rhs);
    }

    BlockVariant *BlockVariant::clone() const {
      return new BlockVariant(*this);
    }

  }  // namespace interface
}  // namespace shared_model
