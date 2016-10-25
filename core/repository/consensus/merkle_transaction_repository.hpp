#ifndef CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_
#define CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_

#include <string>
#include <memory>
#include "../../model/transactions/abstract_transaction.hpp"

namespace merkle_transaction_repository {

bool commit(std::string, const std::unique_ptr<abstract_transaction::AbstractTransaction> &event);

bool leafExists(std::string const hash);
};  // namespace merkle_transaction_repository

#endif  // CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_
