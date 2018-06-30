#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "model_proto.hpp"
#include "model_transaction_builder.hpp"

using namespace emscripten;
using namespace shared_model;
using namespace shared_model::bindings;
using namespace shared_model::proto;

EMSCRIPTEN_BINDINGS(model_transaction_builder) {
  // TODO 20.06.18 stinger112: Add underlaying types for commented methods
  class_<Transaction>("Transaction")
      // .function("commands", &Transaction::commands)
      .function("creatorAccountId", &Transaction::creatorAccountId)
      .function("quorum", &Transaction::quorum)
      .function("blob", &Transaction::blob)
      .function("payload", &Transaction::payload)
      // .function("signatures", &Query::signatures)
      .function("addSignature", &Transaction::addSignature)
      .function("createdTime", &Transaction::getCreatedTime);

  typedef UnsignedWrapper<Transaction> UnsignedWrapper;
  class_<UnsignedWrapper>("UnsignedTx")
      .function("signAndAddSignature", &UnsignedWrapper::signAndAddSignature)
      .function("finish", &UnsignedWrapper::finish)
      .function("hash", &UnsignedWrapper::hash);

  typedef ModelProto<UnsignedWrapper> ModelProto;
  class_<ModelProto>("ModelProtoTransaction")
      .constructor<UnsignedWrapper &>()
      .function("signAndAddSignature", &ModelProto::signAndAddSignature)
      .function("finish", &ModelProto::finish);

  using Role = interface::permissions::Role;
  enum_<Role>("Role")
      .value("kAppendRole", Role::kAppendRole)
      .value("kCreateRole", Role::kCreateRole)
      .value("kDetachRole", Role::kDetachRole)
      .value("kAddAssetQty", Role::kAddAssetQty)
      .value("kSubtractAssetQty", Role::kSubtractAssetQty)
      .value("kAddPeer", Role::kAddPeer)
      .value("kAddSignatory", Role::kAddSignatory)
      .value("kRemoveSignatory", Role::kRemoveSignatory)
      .value("kSetQuorum", Role::kSetQuorum)
      .value("kCreateAccount", Role::kCreateAccount)
      .value("kSetDetail", Role::kSetDetail)
      .value("kCreateAsset", Role::kCreateAsset)
      .value("kTransfer", Role::kTransfer)
      .value("kReceive", Role::kReceive)
      .value("kCreateDomain", Role::kCreateDomain)
      .value("kReadAssets", Role::kReadAssets)
      .value("kGetRoles", Role::kGetRoles)
      .value("kGetMyAccount", Role::kGetMyAccount)
      .value("kGetAllAccounts", Role::kGetAllAccounts)
      .value("kGetDomainAccounts", Role::kGetDomainAccounts)
      .value("kGetMySignatories", Role::kGetMySignatories)
      .value("kGetAllSignatories", Role::kGetAllSignatories)
      .value("kGetDomainSignatories", Role::kGetDomainSignatories)
      .value("kGetMyAccAst", Role::kGetMyAccAst)
      .value("kGetAllAccAst", Role::kGetAllAccAst)
      .value("kGetDomainAccAst", Role::kGetDomainAccAst)
      .value("kGetMyAccDetail", Role::kGetMyAccDetail)
      .value("kGetAllAccDetail", Role::kGetAllAccDetail)
      .value("kGetDomainAccDetail", Role::kGetDomainAccDetail)
      .value("kGetMyAccTxs", Role::kGetMyAccTxs)
      .value("kGetAllAccTxs", Role::kGetAllAccTxs)
      .value("kGetDomainAccTxs", Role::kGetDomainAccTxs)
      .value("kGetMyAccAstTxs", Role::kGetMyAccAstTxs)
      .value("kGetAllAccAstTxs", Role::kGetAllAccAstTxs)
      .value("kGetDomainAccAstTxs", Role::kGetDomainAccAstTxs)
      .value("kGetMyTxs", Role::kGetMyTxs)
      .value("kGetAllTxs", Role::kGetAllTxs)
      .value("kSetMyQuorum", Role::kSetMyQuorum)
      .value("kAddMySignatory", Role::kAddMySignatory)
      .value("kRemoveMySignatory", Role::kRemoveMySignatory)
      .value("kTransferMyAssets", Role::kTransferMyAssets)
      .value("kSetMyAccountDetail", Role::kSetMyAccountDetail)
      .value("kGetBlocks", Role::kGetBlocks);

  using RolePermissionSet = interface::RolePermissionSet;
  class_<RolePermissionSet>("RolePermissionSet")
      .constructor<>()
      .function("size", &RolePermissionSet::size)
      .function("reset", &RolePermissionSet::reset)
      .function("set", &RolePermissionSet::set)
      .function("unset", &RolePermissionSet::unset)
      .function("test", &RolePermissionSet::test)
      .function("none", &RolePermissionSet::none)
      .function("isSubsetOf", &RolePermissionSet::isSubsetOf);

  using Grantable = interface::permissions::Grantable;
  enum_<Grantable>("Grantable")
      .value("kAddMySignatory", Grantable::kAddMySignatory)
      .value("kRemoveMySignatory", Grantable::kRemoveMySignatory)
      .value("kSetMyQuorum", Grantable::kSetMyQuorum)
      .value("kSetMyAccountDetail", Grantable::kSetMyAccountDetail)
      .value("kTransferMyAssets", Grantable::kTransferMyAssets);

  using GrantablePermissionSet = interface::GrantablePermissionSet;
  class_<GrantablePermissionSet>("GrantablePermissionSet")
      .constructor<>()
      .function("size", &GrantablePermissionSet::size)
      .function("reset", &GrantablePermissionSet::reset)
      .function("set", &GrantablePermissionSet::set)
      .function("unset", &GrantablePermissionSet::unset)
      .function("test", &GrantablePermissionSet::test)
      .function("none", &GrantablePermissionSet::none)
      .function("isSubsetOf", &GrantablePermissionSet::isSubsetOf);

  /**
   * Top level ModelTransactionBuilder class
   **/
  class_<ModelTransactionBuilder>("ModelTransactionBuilder")
      .constructor<>()
      .function("creatorAccountId", &ModelTransactionBuilder::creatorAccountId)
      .function(
          "createdTime",
          select_overload<ModelTransactionBuilder(const emscripten::val &)>(
              &ModelTransactionBuilder::createdTime))
      .function("quorum", &ModelTransactionBuilder::quorum)
      .function("addAssetQuantity", &ModelTransactionBuilder::addAssetQuantity)
      .function("addPeer", &ModelTransactionBuilder::addPeer)
      .function("addSignatory", &ModelTransactionBuilder::addSignatory)
      .function("removeSignatory", &ModelTransactionBuilder::removeSignatory)
      .function("appendRole", &ModelTransactionBuilder::appendRole)
      .function("createAsset", &ModelTransactionBuilder::createAsset)
      .function("createAccount", &ModelTransactionBuilder::createAccount)
      .function("createDomain", &ModelTransactionBuilder::createDomain)
      .function("createRole", &ModelTransactionBuilder::createRole)
      .function("detachRole", &ModelTransactionBuilder::detachRole)
      .function("grantPermission", &ModelTransactionBuilder::grantPermission)
      .function("revokePermission", &ModelTransactionBuilder::revokePermission)
      .function("setAccountDetail", &ModelTransactionBuilder::setAccountDetail)
      .function("setAccountQuorum", &ModelTransactionBuilder::setAccountQuorum)
      .function("subtractAssetQuantity",
                &ModelTransactionBuilder::subtractAssetQuantity)
      .function("transferAsset", &ModelTransactionBuilder::transferAsset)
      .function("build", &ModelTransactionBuilder::build);
}
