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

#include "validator.hpp"
#include <commands_generated.h>
#include <transaction_generated.h>
#include <ametsuchi/ametsuchi.h>
#include <ametsuchi/repository.hpp>
#include <tuple>

namespace runtime {
    namespace validator {

        bool account_exist_validator(const flatbuffers::String &publicKey){
            return repository::existAccountOf(publicKey);
        }

        auto prev_validator = [](
            const flatbuffers::String& publicKey,
            const flatbuffers::String& target_ledger,
            const flatbuffers::String& target_domain,
            const flatbuffers::String& target_asset
        ) -> std::function<bool(const ::iroha::Command)> {
            auto asset_permissions  = repository::permission::getPermissionAssetOf(publicKey);
            for(const iroha::AccountPermissionAsset* ap: asset_permissions) {
                if (
                    ap->asset_name()->str()  == target_asset.str() &&
                    ap->domain_name()->str() == target_domain.str() &&
                    ap->ledger_name()->str() == target_ledger.str()
                ){
                    return [=](const iroha::Command c) -> bool{
                        return (ap->read()  && (
                            (ap->transfer() && iroha::Command::Transfer == c ) ||
                            (ap->add()      && iroha::Command::Add == c ) ||
                            (ap->subtract() && iroha::Command::Subtract == c )
                        ));
                    };
                }
            }
            return [](const iroha::Command c) -> bool{
                return false;
            };
        };

        auto getUrlFromAsse = [](const iroha::Asset* asset) ->
            std::tuple<
                const flatbuffers::String*,
                const flatbuffers::String*,
                const flatbuffers::String*
            > {
            switch(asset->asset_type()){
                case iroha::AnyAsset::ComplexAsset: {
                    return {
                            asset->asset_as_ComplexAsset()->ledger_name(),
                            asset->asset_as_ComplexAsset()->domain_name(),
                            asset->asset_as_ComplexAsset()->asset_name()
                    };
                }
                case iroha::AnyAsset::Currency:{
                    return {
                        asset->asset_as_Currency()->ledger_name(),
                        asset->asset_as_Currency()->domain_name(),
                        asset->asset_as_Currency()->currency_name()
                    };
                }
                case iroha::AnyAsset::NONE: throw; // ToDo
            }
        };

        bool permission_validator(const iroha::Transaction& tx){
            const flatbuffers::String* ledger_name = nullptr;
            const flatbuffers::String* domain_name = nullptr;
            const flatbuffers::String* asset_name  = nullptr;
            switch(tx.command_type()) {
                case iroha::Command::Add: {
                    std::tie(ledger_name,domain_name,asset_name) =
                        getUrlFromAsse(tx.command_as_Add()->asset_nested_root());
                }break;
                case iroha::Command::Subtract: {
                    std::tie(ledger_name,domain_name,asset_name) =
                            getUrlFromAsse(tx.command_as_Add()->asset_nested_root());
                }break;
                case iroha::Command::Transfer: {
                    std::tie(ledger_name,domain_name,asset_name) =
                            getUrlFromAsse(tx.command_as_Add()->asset_nested_root());
                }break;
                case iroha::Command::AssetCreate:           return true;
                case iroha::Command::AssetRemove:           return true;
                case iroha::Command::PeerAdd:               return true;
                case iroha::Command::PeerRemove:            return true;
                case iroha::Command::PeerSetActive:         return true;
                case iroha::Command::PeerSetTrust:          return true;
                case iroha::Command::PeerChangeTrust:       return true;
                case iroha::Command::AccountAdd:            return true;
                case iroha::Command::AccountRemove:         return true;
                case iroha::Command::AccountAddSignatory:   return true;
                case iroha::Command::AccountRemoveSignatory:return true;
                case iroha::Command::AccountSetUseKeys:     return true;
                case iroha::Command::AccountMigrate:        return true;
                case iroha::Command::ChaincodeAdd:          return true;
                case iroha::Command::ChaincodeRemove:       return true;
                case iroha::Command::ChaincodeExecute:      return true;
                case iroha::Command::PermissionRemove:      return true;
                case iroha::Command::PermissionAdd:         return true;
                case iroha::Command::NONE:                  break;
            }

            if(ledger_name  == nullptr  ||
                domain_name == nullptr ||
                asset_name  == nullptr
            ) return false;

            return prev_validator(
                *tx.creatorPubKey(),
                *ledger_name,
                *domain_name,
                *asset_name
            )(tx.command_type());
        }

        bool logic_validator(const iroha::Transaction &tx){

        }

    };

};