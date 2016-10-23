#include "consensus_event_validator.hpp"
#include "../consensus/consensus_event.hpp"

#include "../crypto/signature.hpp"

namespace consensus_event_validator {

bool isValid(const consensus_event::ConsensusEvent event) {
    return true;//signaturesAreValid; // TODO: add more tests
}

bool isValid(std::string sig) {
    return signaturesAreValid; // TODO: add more tests
}


bool signaturesAreValid(const consensus_event::ConsensusEvent event) {
    for (auto sig : event.signatures) {
        if (!consensus_event_validator::isValid(sig)) {
            return false;
        }
    }
    return true;
}

};  // namespace consensus_event_validator
