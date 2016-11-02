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

#ifndef __CORE_REPOSITORY_DOMAIN_SAMPLE_ASSET_REPOSITORY_HPP__
#define __CORE_REPOSITORY_DOMAIN_SAMPLE_ASSET_REPOSITORY_HPP__

#include <string>
#include <memory>
#include <vector>

#include "../../model/state/sample_asset.hpp"

// I know only 'domain' and 'asset'.
namespace domain_sample_asset_repository {
    
    bool alreadyExists(std::string domainName);
    
    bool registerDomain(
        std::string accountUid,
        std::string domainName
    );

    bool thisAssetIsInDomain(
        std::string assetName,
        std::string domainName
    );

    bool join(
        const domain::asset::SampleAsset& asset,
        std::string domainName
    );
};
#endif  // CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_
