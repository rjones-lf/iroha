#ifndef CORE_CONSENSUS_CONSENSUSEVENT_HPP_
#define CORE_CONSENSUS_CONSENSUSEVENT_HPP_

#include <string>
#include <functional>
#include <memory>
#include <type_traits>

#include "../model/transactions/abstract_transaction.hpp"

namespace consensus_event {

struct ConsensusEvent {

    std::unique_ptr<abstract_transaction::AbstractTransaction> tx;
    std::vector<std::string> signatures;
    std::string merkleRoot;

    unsigned long long order = 0;
    std::vector<std::string> merkleRootSignatures;

    ConsensusEvent(std::unique_ptr<abstract_transaction::AbstractTransaction> atx):
        tx(std::move(atx))
    {}

    ConsensusEvent():
            tx(nullptr)
    {}

    void addSignature(const std::string& signature){
        signatures.push_back(signature);
    }

    std::string getHash() const {
        return tx->getHash();
    }

    operator std::string() const{
        return "WIP";
    }
};
};  // namespace ConsensusEvent

#endif  // CORE_CONSENSUS_CONSENSUSEVENT_HPP_