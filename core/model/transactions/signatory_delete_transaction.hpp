#ifndef CORE_DOMAIN_TRANSACTIONS_SIGNATORYDELETETRANSACTION_HPP_
#define CORE_DOMAIN_TRANSACTIONS_SIGNATORYDELETETRANSACTION_HPP_

#include "abstract_transaction.hpp"
#include <msgpack.hpp>

namespace signatory_delete_transaction {
    MSGPACK_DEFINE(hash, type, accountPublicKey, signerToDeletePublicKey, signature, timestamp);
    std::string getJSON();
};  // namespace signatory_delete_transaction

#endif  // CORE_DOMAIN_TRANSACTIONS_SIGNATORYDELETETRANSACTION_HPP_
