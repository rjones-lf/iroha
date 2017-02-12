/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
/*
  This code is used for creating builder header file only.
*/
#define REPLACE_STRING(str) #str

#define BUILDER_NAMESPACE_BEGIN namespace transaction {
#define BUILDER_BEGIN(Command, ObjectType) \
template<>  \
class TransactionBuilder<type_signatures::Command<object::ObjectType>> { \
public: \
  \
  TransactionBuilder() = default; \
  TransactionBuilder(const TransactionBuilder&) = default;  \
  TransactionBuilder(TransactionBuilder&&) = default;

#define BUILDER_SET_SENDER \
  TransactionBuilder& setSender(std::string sender) { \
    if (_isSetSender) { \
      throw std::domain_error(std::string("Duplicate sender in ") + __FILE__); \
    } \
    _isSetSender = true;  \
    _sender = std::move(sender);  \
    return *this; \
  }

#define BUILDER_SET_OBJECT(ObjectType, objectType)  \
  TransactionBuilder& set ## ObjectType(object::ObjectType object) {  \
    if (_isSet##ObjectType) { \
      throw std::domain_error(std::string("Duplicate ") + #ObjectType + " in " + __FILE__); \
    } \
    _isSet##ObjectType = true;  \
    _##objectType = std::move(object);  \
    return *this; \
  }

#define BUILDER_BUILD(Command, ObjectType, objectType)  \
  transaction::Transaction build() {  \
    const auto unsetMembers = enumerateUnsetMembers();  \
    if (not unsetMembers.empty()) { \
      throw exception::transaction::UnsetBuildArgmentsException(REPLACE_STRING(Command##<object::##ObjectType##>), unsetMembers); \
    } \
    \
    return transaction::Transaction(_sender, command::Command(_##objectType)); \
  }

#define BUILDER_UNSET_MEMBERS_BEGIN(ObjectType) \
private:  \
  \
  std::string enumerateUnsetMembers() {    \
    std::string ret;  \
    if (not _isSetSender) ret += std::string(" ") + "sender";  \
    if (not _isSet##ObjectType) ret += std::string(" ") + #ObjectType;

#define BUILDER_UNSET_MEMBERS_CUSTOM_MEMBER(member, defaultCondition) \
    if (not _ ## member ## defaultCondition) ret += std::string(" ") + REPLACE_STRING(member);

#define BUILDER_UNSET_MEMBERS_END \
    return ret; \
  }

#define BUILDER_MEMBERS(ObjectType, objectType) \
  std::string _sender;  \
  object::ObjectType _##objectType;

#define BUILDER_MEMBERS_BOOL(ObjectType) \
  bool _isSetSender = false; \
  bool _isSet##ObjectType = false;

#define BUILDER_CUSTOM_MEMBER(type,name) \
  type _##name;

#define BUILDER_CUSTOM_MEMBER_BOOL(capitalizedName) \
  bool _isSet##capitalizedName = false;

#define BUILDER_END  \
};
#define BUILDER_NAMESPACE_END \
}
BUILDER_NAMESPACE_BEGIN

BUILDER_BEGIN(__CommandType__,__ObjectType__)

BUILDER_SET_SENDER

BUILDER_SET_OBJECT(__ObjectType__,__objectType__)

BUILDER_BUILD(__CommandType__,__ObjectType__,__objectType__)

BUILDER_UNSET_MEMBERS_BEGIN(__ObjectType__)
BUILDER_UNSET_MEMBERS_END

BUILDER_MEMBERS(__ObjectType__,__objectType__)

BUILDER_MEMBERS_BOOL(__ObjectType__)

BUILDER_END
BUILDER_NAMESPACE_END
/*
Require: clang-format
Preprocessing error will occur but the output code is correct.
*/