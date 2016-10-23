#include "domain_renewal_transaction.hpp"

namespace domain_renewal_transaction {
    class DomainRenewalTransaction : public AbstractTransaction {
        std::string hash;
        AbstractTransaction::TransactionType type;

        std::string getHash() {
            return hash;
        }

        std::string getRawData() {
            //TODO
        }

        std::string getAsText() {
            //TODO
        }

        unsigned long long getTimestamp() {
            return timestamp;
        }

        TransactionType getType() {
            return type;
        }
    };

    namespace domain_renewal_transaction {
    };  // namespace DomainRenewalTransaction
}
