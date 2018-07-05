/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

%module iroha

#define DEPRECATED
#define FINAL
#pragma SWIG nowarn=325, 401, 509, 516

%include "std_string.i"
%include "stdint.i"
%include "exception.i"
%include "std_vector.i"

%{
#include "cryptography/hash.hpp"
#include "interfaces/common_objects/signature.hpp"
%}

namespace std {
  %template(ByteVector) vector<uint8_t>;
  %template(StringVector) vector<string>;
  %ignore vector<shared_model::crypto::Hash>::vector(size_type);
  %template(HashVector) vector<shared_model::crypto::Hash>;
  %template(IntVector) vector<int>;
  %template(SignatureVector) vector<const shared_model::interface::Signature*>;
};

%exception {
  try {
    $action
  } catch (const std::invalid_argument &e) {
    SWIG_exception(SWIG_ValueError, e.what());
  } catch (const std::out_of_range &e) {
    SWIG_exception(SWIG_OverflowError, e.what());
  }
}

%ignore shared_model::proto::Query::get;
%ignore shared_model::proto::Transaction::commands;
%ignore shared_model::proto::Query::signatures;
%ignore shared_model::proto::BlocksQuery::signatures;
%ignore shared_model::proto::Transaction::signatures;

%extend shared_model::proto::Query {
  std::vector<const shared_model::interface::Signature*> signs() {
    std::vector<const shared_model::interface::Signature*> sigs;

    for (const auto &s : $self->signatures()) {
      sigs.push_back(&s);
    }
    return sigs;
  }
}

%extend shared_model::proto::BlocksQuery {
  std::vector<const shared_model::interface::Signature*> signs() {
    std::vector<const shared_model::interface::Signature*> sigs;

    for (const auto &s : $self->signatures()) {
      sigs.push_back(&s);
    }
    return sigs;
  }
}

%extend shared_model::proto::Transaction {
  std::vector<const shared_model::interface::Signature*> signs() {
    std::vector<const shared_model::interface::Signature*> sigs;

    for (const auto &s : $self->signatures()) {
      sigs.push_back(&s);
    }
    return sigs;
  }
}

%rename(ModelTransaction) iroha::protocol::Transaction;
%rename(_interface) interface;
%rename(b_equal) shared_model::crypto::Blob::operator==;
%rename(kp_equal) shared_model::crypto::Keypair::operator==;
%rename(hash_invoke) shared_model::crypto::Hash::Hasher::operator();
%rename(Signature) shared_model::interface::Signature;

%ignore shared_model::interface::PermissionSet::PermissionSet(std::initializer_list<shared_model::interface::permissions::Role>);
%ignore shared_model::interface::PermissionSet::PermissionSet(std::initializer_list<shared_model::interface::permissions::Grantable>);
%rename(bset_and) shared_model::interface::PermissionSet::operator&=;
%rename(bset_or) shared_model::interface::PermissionSet::operator|=;
%rename(bset_xor) shared_model::interface::PermissionSet::operator^=;

#if defined(SWIGJAVA) || defined(SWIGJAVASCRIPT)
%rename(equal) operator==;
%rename(not_equal) operator!=;
#endif

%extend shared_model::interface::PermissionSet {
  PermissionSet(const std::vector<int> &perms) {
    shared_model::interface::PermissionSet<Perm> *set = new shared_model::interface::PermissionSet<Perm>;
    for (auto p: perms) {
      set->set(static_cast<Perm>(p));
    }
    return set;
  }
}

%{
#include "bindings/model_transaction_builder.hpp"
#include "bindings/model_query_builder.hpp"
#include "bindings/model_blocks_query_builder.hpp"
#include "bindings/model_crypto.hpp"
#include "bindings/model_proto.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "bindings/client_api.hpp"
%}

%include "cryptography/blob.hpp"
%include "interfaces/common_objects/types.hpp"
%include "interfaces/common_objects/signature.hpp"
%include "interfaces/base/signable.hpp"
%include "interfaces/permissions.hpp"
%include "cryptography/public_key.hpp"
%include "cryptography/private_key.hpp"
%include "cryptography/hash.hpp"
%include "cryptography/keypair.hpp"
%include "cryptography/signed.hpp"
%include "backend/protobuf/transaction.hpp"
%include "backend/protobuf/queries/proto_query.hpp"
%include "backend/protobuf/queries/proto_blocks_query.hpp"

%include "builders/protobuf/unsigned_proto.hpp"
%include "bindings/model_transaction_builder.hpp"
%include "bindings/model_query_builder.hpp"
%include "bindings/model_blocks_query_builder.hpp"
%include "bindings/model_crypto.hpp"
%include "bindings/model_proto.hpp"
%include "bindings/client_api.hpp"

%template (UnsignedTx) shared_model::proto::UnsignedWrapper<shared_model::proto::Transaction>;
%template (UnsignedQuery) shared_model::proto::UnsignedWrapper<shared_model::proto::Query>;
%template (UnsignedBlockQuery) shared_model::proto::UnsignedWrapper<shared_model::proto::BlocksQuery>;
%template (ModelProtoTransaction) shared_model::bindings::ModelProto<shared_model::proto::UnsignedWrapper<shared_model::proto::Transaction>>;
%template (ModelProtoQuery) shared_model::bindings::ModelProto<shared_model::proto::UnsignedWrapper<shared_model::proto::Query>>;
%template (ModelProtoBlocksQuery) shared_model::bindings::ModelProto<shared_model::proto::UnsignedWrapper<shared_model::proto::BlocksQuery>>;
%template (RolePermissionSet) shared_model::interface::PermissionSet<shared_model::interface::permissions::Role>;
%template (GrantablePermissionSet) shared_model::interface::PermissionSet<shared_model::interface::permissions::Grantable>;
