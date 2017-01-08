

#include "consensus_event.hpp"

namespace event{
    
    template<typename T>
    using Transaction = transaction::Transaction<T>;
    template<typename T>
    using Transfer = command::Transfer<T>;
    template<typename T>
    using Add = command::Add<T>;

    template<typename T>
    using Update = command::Update<T>;

    template <>
    void ConsensusEvent<Transaction<Update<object::Asset>>>::execution(){
        LOG_INFO("execution") << "update! Asset";
    }
    template <>
    void ConsensusEvent<Transaction<Add<object::Asset>>>::execution(){
        LOG_INFO("execution") << "add! Asset";
    }
    template <>
    void ConsensusEvent<Transaction<Add<object::Domain>>>::execution(){
        LOG_INFO("execution") << "add! Asset";
    }

};