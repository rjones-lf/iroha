

#include "consensus_event.hpp"

namespace event{
    
    template<typename T>
    using Transaction = transaction::Transaction<T>;
    template<typename T>
    using Transfer = command::Transfer<T>;
    template<typename T>
    using Add = command::Add<T>;

    using Type = json_parse::Type;
    using Object = json_parse::Object;
    
    template <>
    ConsensusEvent<Transaction<Transfer<object::Asset>>>::ConsensusEvent(
        Object obj
    ):
        Transaction(obj)
    {
        order = obj.dictSub["order"].floating;
        std::vector<Object> eventSigs = obj.dictSub["eventSignatures"].listSub;
        for(auto&& sig : eventSigs){
            eventSignatures.push_back(eventSignature(sig.dictSub["publicKey"].str,sig.dictSub["signature"].str));
        }
    }

    template <>
    ConsensusEvent<Transaction<Add<object::Asset>>>::ConsensusEvent(
        Object obj
    ):
        Transaction(obj)
    {
        order = obj.dictSub["order"].floating;
        std::vector<Object> eventSigs = obj.dictSub["eventSignatures"].listSub;
        for(auto&& sig : eventSigs){
            std::cout <<"construct:"<< sig.dictSub["publicKey"].str <<std::endl;
            eventSignatures.push_back(eventSignature(sig.dictSub["publicKey"].str,sig.dictSub["signature"].str));
        }
    }

    template <>
    ConsensusEvent<Transaction<Add<object::Domain>>>::ConsensusEvent(
        Object obj
    ):
        Transaction(obj)
    {
        order = obj.dictSub["order"].floating;
        std::vector<Object> eventSigs = obj.dictSub["eventSignatures"].listSub;
        for(auto&& sig : eventSigs){
            eventSignatures.push_back(eventSignature(sig.dictSub["publicKey"].str,sig.dictSub["signature"].str));
        }
    }


    template <>
    ConsensusEvent<Transaction<Transfer<object::Asset>>>::ConsensusEvent(
        const std::string& senderPubkey,
        const std::string& receiverPubkey,
        const std::string& name,
        const int& value
    ):
        Transaction(
            senderPubkey,
            receiverPubkey,
            name,
            value
        )
    {}

    template <>
    ConsensusEvent<Transaction<Add<object::Asset>>>::ConsensusEvent(
        const std::string& senderPubkey,
        const std::string& domain,
        const std::string& name,
        const unsigned long long& value,
        const unsigned int& precision
    ):
        Transaction(
            senderPubkey,
            domain,
            name,
            value,
            precision
        )    
    {}

    template <>
    ConsensusEvent<Transaction<Add<object::Domain>>>::ConsensusEvent(
        const std::string& senderPubkey,
        const std::string& ownerPublicKey,
        const std::string& name
    ):
        Transaction(
            senderPubkey,
            ownerPublicKey,
            name
        )    
    {}

};