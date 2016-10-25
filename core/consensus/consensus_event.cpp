#include "consensus_event.hpp"

#include "../model/transactions/abstract_transaction.hpp"

namespace consensus_event {

    void ConsensusEvent::addSignature(const std::string& signature) {
        signatures.push_back(signature); //TODO: use std::move here? Makoto: probably no
    }

    std::string ConsensusEvent::getHash() const {
        return tx->getHash();
    }
}  // namespace consensus_event
