#ifndef CORE_DOMAIN_TRANSACTIONS_TRANSFERTRANSACTION_HPP_
#define CORE_DOMAIN_TRANSACTIONS_TRANSFERTRANSACTION_HPP_

#include "abstract_transaction.hpp"

namespace transaction {

class TransferTransaction : public abstract_transaction::AbstractTransaction {
    std::string prevTxHash;
    std::string hash;
    abstract_transaction::TransactionType type;
    std::string senderPublicKey;
    std::string receiverPublicKey;
    std::string domain;
    std::string asset;
    long long makotos;  // TODO: JS NUMBER range from -9007199254740992 to +9007199254740992 対応
    short int precision;
    unsigned long long timestamp;

public:
    TransferTransaction(abstract_transaction::AbstractTransaction &&, const std::string &prevTxHash);

private:
    std::string getHash();
    std::string getRawData();
    std::string getAsText();
    unsigned long long  getTimestamp();
    abstract_transaction::TransactionType getType();
};

};  // namespace transaction

#endif  // CORE_DOMAIN_TRANSACTIONS_TRANSFERTRANSACTION_HPP_
