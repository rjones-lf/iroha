#include "transaction_validator.hpp"

#include "../model/transactions/abstract_transaction.hpp"
#include "../model/transactions/transfer_transaction.hpp"
#include "../crypto/signature.hpp"

namespace transaction_validator {
bool isValid(const abstract_transaction::AbstractTransaction& tx) {
    return signaturesAreValid(tx) && validForType(tx);
}

bool signaturesAreValid(const abstract_transaction::AbstractTransaction& tx) {
    if (!signature::verify(tx.signature, tx.signature, tx.publicKey)) {
        return true;
    } else {
        return false;
    }
}

bool validForType(const abstract_transaction::AbstractTransaction& tx) {
    if (abstract_transaction::TransactionType::transfer == tx.getType()) {
       // transaction::TransferTransaction transferTx = static_cast<transaction::TransferTransaction>(tx);
    } else if (abstract_transaction::TransactionType::addPeer == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::modifyPeer == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::removePeer == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::signatory == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::signatoryAdd == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::signatoryDelete == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::domainDefinition == tx.getType()) {

    } else if (abstract_transaction::TransactionType::domainRenewal == tx.getType()) {

    } else if (abstract_transaction::TransactionType::aliasDefinition == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::aliasRenewal == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::assetDefinition == tx.getType()) {

    } else if (abstract_transaction::TransactionType::message == tx.getType()) {

    } else if (abstract_transaction::TransactionType::chaincodeInit == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::chaincodeInvoke == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::chaincodeUpdate == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::chaincodeDestroy == tx.getType()) {
        return true;  //TODO-future-work
    } else if (abstract_transaction::TransactionType::interchain == tx.getType()) {
        return true;  //TODO-future-work
    }
    return false;
}

};  // namespace transaction_validator
