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

#include <string>
#include <vector>

#include "account.hpp"

// Include some service
#include "../../service/url_service.hpp"

// Include asset repository
#include "../../repository/domain/domain_sample_asset_repository.hpp"
#include "../../repository/domain/account_repository.hpp"

// Include asset publisher
#include "../../publisher/asset_publisher.hpp"

#include <vector>

namespace domain {

    // Umm, this should return whether registration failed or already exists?
    // Personally, this should only return whether registration failed or not.
    // この関数は登録失敗か既に存在しているかの状態を伝えたほうが良い？
    // 個人的には登録したかどうかを返すのがこの関数の責務だと思うが。
    bool AccountUser::registerDomain(const std::string &domainName) {
        if(domain_sample_asset_repository::alreadyExists(domainName)) {
            return domain_sample_asset_repository::registerDomain(
                this->uid,
                domainName
            );
        }else{
            return false;
        }
    }

    bool AccountUser::isOwnerOfDomain(const std::string& domainName) {
        // not exist == domain nobody has
        if (domain_sample_asset_repository::alreadyExists(domainName)) {
            for (auto& domain : this->hasDomainNames) {
                if (domain == domainName) {
                     return true;
                }
            }   
            return false;
        } else {
            return false;
        }
    }

    // Umm, this should return whether registration failed or already exists?
    // Personally, this should only return whether registration failed or not.
    // 上と同じく。
    bool AccountUser::joinSampleAssetTo(const asset::SampleAsset &sampleAsset, const std::string &domain) {
        if (!domain_sample_asset_repository::thisAssetIsInDomain(sampleAsset.name, domain)) {
            return domain_sample_asset_repository::join(sampleAsset, domain);
        } else {
            return false;
        }
    }

    bool AccountUser::pay(const std::string &to, const int quantity, const std::string &assetUrl) {
        std::unique_ptr<AccountUser> receiverAccount = account_repository::findByUid(to);
        if (receiverAccount != nullptr) {
           
            // Oh no...,This is bad. not only asset name, require domain information.
            // Todo use domain informaion
            std::pair<
                std::vector<std::string>,
                std::string
            > assetName = service::url_service::getAssetNameFromUrl(assetUrl);
            int myBalance = this->sampleAssetQuantitiesWhatIHaveAccount.at(assetName.second);
            int receiverBalance = receiverAccount->sampleAssetQuantitiesWhatIHaveAccount.at(assetName.second);

            if (myBalance > quantity) {
                account_repository::update_quantity( 
                    receiverAccount->uid,
                    receiverBalance + quantity,
                    assetName.second
                );

                account_repository::update_quantity( 
                    this->uid,
                    myBalance - quantity,
                    assetName.second
                );      

                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }    
}
