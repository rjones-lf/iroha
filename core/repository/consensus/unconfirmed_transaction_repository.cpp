
#include <string>
#include "../../model/transactions/abstract_transaction.hpp"

namespace unconfirmed_transaction_repository {
  bool add(std::string hash, const abstract_transaction::AbstractTransaction& tx){
      return false;
  }
  bool update(std::string hash, const abstract_transaction::AbstractTransaction& tx){
      return false;
  }
  bool remove(std::string hash){}

  abstract_transaction::AbstractTransaction& find(std::string hash){

  }
};  // namespace unconfirmed_transaction_repository
