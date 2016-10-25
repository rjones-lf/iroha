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

std::vector<std::unique_ptr<abs_tx>> transactions;

// WIP
//std::unique_ptr<merkle::MerkleRoot> merkle_root;

bool commit(std::string hash, const std::unique_ptr<abs_tx> &tx) {
    return repository::world_state_repository::add(tx->getHash(), tx->getAsText());
}

bool leafExists(std::string const hash) {
    return !repository::world_state_repository::find(hash).empty();
}

};  // namespace merkle_transaction_repository
