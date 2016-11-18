

#include "transaction.hpp"

namespace transaction{

    template <>
    Transaction<Transfer<object::Asset>>::Transaction(
        const std::string& senderPubkey,
        const std::string& receiverPubkey,
        const std::string& name,
        const int& value
    ):
        senderPubkey(senderPubkey),
        Transfer(
            senderPubkey,
            receiverPubkey,
            std::string name,
            value
        )
    {}

    template <>
    Transaction<Add<object::Asset>>::Transaction(
        const std::string& senderPubkey,
        const std::string& domain,
        const std::string& name,
        const unsigned long long& value,
        const unsigned int& precision
    ):
        senderPubkey(senderPubkey),
        Add(
            domain,
            name,
            value,
            precision
        )    
    {}

    template <>
    Transaction<Add<object::Domain>>::Transaction(
        const std::string& senderPubkey,
        const std::string& ownerPublicKey,
        const std::string& name
    ):
        senderPubkey(senderPubkey),
        Add(
            ownerPublicKey,
            name
        )    
    {}

}