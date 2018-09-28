/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_COMMAND_HPP
#define IROHA_SHARED_MODEL_COMMAND_HPP

#include <boost/variant.hpp>
#include "interfaces/base/model_primitive.hpp"

namespace shared_model {
  namespace interface {

    class AddAssetQuantity;
    class AddPeer;
    class AddSignatory;
    class AppendRole;
    class CreateAccount;
    class CreateAsset;
    class CreateDomain;
    class CreateRole;
    class DetachRole;
    class GrantPermission;
    class RemoveSignatory;
    class RevokePermission;
    class SetAccountDetail;
    class SetQuorum;
    class SubtractAssetQuantity;
    class TransferAsset;

    /**
     * Class provides commands container for all commands in system.
     * General note: this class is container for commands, not a base class.
     */
    class Command : public ModelPrimitive<Command> {
     private:
      /// const reference shortcut type
      template <typename... Value>
      using wrap = boost::variant<const Value &...>;

     public:
      /// Type of variant, that handle concrete command
      using CommandVariantType = wrap<AddAssetQuantity,
                                      AddPeer,
                                      AddSignatory,
                                      AppendRole,
                                      CreateAccount,
                                      CreateAsset,
                                      CreateDomain,
                                      CreateRole,
                                      DetachRole,
                                      GrantPermission,
                                      RemoveSignatory,
                                      RevokePermission,
                                      SetAccountDetail,
                                      SetQuorum,
                                      SubtractAssetQuantity,
                                      TransferAsset>;

      /// Types of concrete commands, in attached variant
      using CommandListType = CommandVariantType::types;

      /**
       * @return reference to const variant with concrete command
       */
      virtual const CommandVariantType &get() const = 0;

      // ------------------------| Primitive override |-------------------------

      std::string toString() const override;

      bool operator==(const ModelType &rhs) const override;
    };

  }  // namespace interface
}  // namespace shared_model

namespace boost {
  extern template class variant<
      const shared_model::interface::AddAssetQuantity &,
      const shared_model::interface::AddPeer &,
      const shared_model::interface::AddSignatory &,
      const shared_model::interface::AppendRole &,
      const shared_model::interface::CreateAccount &,
      const shared_model::interface::CreateAsset &,
      const shared_model::interface::CreateDomain &,
      const shared_model::interface::CreateRole &,
      const shared_model::interface::DetachRole &,
      const shared_model::interface::GrantPermission &,
      const shared_model::interface::RemoveSignatory &,
      const shared_model::interface::RevokePermission &,
      const shared_model::interface::SetAccountDetail &,
      const shared_model::interface::SetQuorum &,
      const shared_model::interface::SubtractAssetQuantity &,
      const shared_model::interface::TransferAsset &>;
}  // namespace boost

#endif  // IROHA_SHARED_MODEL_COMMAND_HPP
