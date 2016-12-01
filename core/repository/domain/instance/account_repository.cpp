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

#include "../../../model/state/account.hpp"
#include "../account_repository.hpp"
#include "../../world_state_repository.hpp"

    namespace repository{
    namespace account {

        std::unique_ptr<domain::AccountUser> convertAccountUser(const std::string &buffer) {
            return nullptr; // WIP
        }

        std::string convertBuffer(const std::unique_ptr<domain::AccountUser>& au) {
            return ""; // WIP
        }

        // SampleAsset has only quantity no logic, so this value is int.
        bool update_quantity(
            std::string accountUid,
            int newValue,
            std::string assetName) {
            std::unique_ptr<domain::AccountUser> accountUser = convertAccountUser(
                world_state_repository::find(accountUid)
            );
            if(
                accountUser->sampleAssetQuantitiesWhatIHaveAccount.find(assetName) !=
                accountUser->sampleAssetQuantitiesWhatIHaveAccount.end()
            ){
                accountUser->sampleAssetQuantitiesWhatIHaveAccount.at(assetName) = newValue;
                return world_state_repository::update(accountUid, convertBuffer(std::move(accountUser)));
            } else {
                return false;
            }
        }

        std::unique_ptr<domain::AccountUser> findByUid(std::string accountUid) {
            return convertAccountUser(
                world_state_repository::find(accountUid)
            );
        }

        bool add_my_domain(
            const std::string& accountUid,
            const std::string& domainName
        ){
            std::unique_ptr<domain::AccountUser> accountUser = convertAccountUser(
                world_state_repository::find(accountUid)
            );
            accountUser->hasDomainNames.push_back(domainName);
            return world_state_repository::update(accountUid, convertBuffer(std::move(accountUser)));
        }

    };
};