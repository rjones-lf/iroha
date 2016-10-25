#include "transfer_transaction.hpp"
#include <string>

namespace transaction {

    TransferTransaction::TransferTransaction(
         const std::string &senderPublicKey, const std::string &receiverPublicKey,
         const std::string &domain, const std::string &asset
    ):
        AbstractTransaction(),
        senderPublicKey(senderPublicKey),
        receiverPublicKey(receiverPublicKey),
        domain(domain),
        asset(asset)
    {}

    std::string TransferTransaction::getHash() const {
        return "";
    }

    std::string TransferTransaction::getRawData() const {
        return "";
    }

    std::string TransferTransaction::getAsText() const {
        return "";
    }

    unsigned long long int TransferTransaction::getTimestamp() const {
        return 0;
    }

    abstract_transaction::TransactionType TransferTransaction::getType() const {
        return abstract_transaction::TransactionType::transfer;
    }

};  // namespace transaction
