#ifndef CORE_DOMAIN_ACCOUNT_HPP_
#define CORE_DOMAIN_ACCOUNT_HPP_

#include <string>
#include <vector>

#include "sample_asset.hpp"
#include "../util/random.hpp"

#include <vector>
#include <string>
#include <unordered_map>

namespace domain {

  // This is domain.
  class AccountUser {
      std::string name;
      std::string publicKeyb64Encoded;
      std::string uid;

    public:
      // Account user can have some domain. 
      std::vector<std::string> hasDomainNames;

      // SampleAsset has only quantity no logic, so this value is int.
      std::unordered_map<
        std::string,
        int
      > sampleAssetQuantitiesWhatIHaveAccount;

      // This user created asset. 
      std::vector<std::string> myAssetNames;


      // Use only factory.
      AccountUser(
        std::string aName,
        std::string aPublicKeyb64Encoded
      ):
        name(aName),
        publicKeyb64Encoded(aPublicKeyb64Encoded),
        uid(random_service::makeRandomHash())
      {}

      // Use convertor
      AccountUser(
              std::string aName,
              std::string aPublicKeyb64Encoded,
              std::string aUid
      ):
              name(aName),
              publicKeyb64Encoded(aPublicKeyb64Encoded),
              uid(aUid)
      {}


      // Support move and copy.
      AccountUser(AccountUser const&) = default;
      AccountUser(AccountUser&&) = default;
      AccountUser& operator =(AccountUser const&) = default;
      AccountUser& operator =(AccountUser&&) = default;

      // Account user can register a domain and has some a domain;
      bool registerDomain(const std::string &domainName);
      bool isOwnerOfDomain(const std::string& domainName);

      // Account user can add an asset to the domain if account user has this domain;
      bool joinSampleAssetTo(
        const asset::SampleAsset &sampleAsset,
        const std::string &domain);

      // Account user can pay other user in any asset;
      // assetUrl := domain::domain::asset.
      // Of cause, account balance must not minus.
      bool pay(
        const std::string &to,
        int quantity,
        const std::string &assetUrl);
      
  };
}
#endif  // CORE_DOMAIN_ACCOUNT_HPP_
