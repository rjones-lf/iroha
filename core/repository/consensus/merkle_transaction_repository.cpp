#include "merkle_transaction_repository.hpp"
#include "../world_state_repository.hpp"

#include <string>
#include <memory>
#include <iostream>

#include "../../util/logger.hpp"
#include "../../crypto/merkle_node.hpp"
#include "../../crypto/merkle.hpp"

namespace merkle_transaction_repository {

using abs_tx = abstract_transaction::AbstractTransaction;

// WIP
//std::unique_ptr<merkle::MerkleRoot> merkle_root;

bool commit(std::unique_ptr<consensus_event::ConsensusEvent> const event) {
    return false;
}

bool commit(std::string hash, std::unique_ptr<consensus_event::ConsensusEvent> const event) {
    return false;
}

std::unique_ptr<abs_tx> findLeaf(std::string const hash) {

}

};  // namespace merkle_transaction_repository
