
#include "../../../model/account.hpp"
#include "../account_repository.hpp"
#include "../../world_state_repository.hpp"

namespace repository{
namespace account_repository {

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