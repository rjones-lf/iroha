
#include "event_repository.hpp"
#include "../world_state_repository.hpp"
#include <string>

#include "../../util/convert_string.hpp"
#include "../../model/transactions/abstract_transaction.hpp"

namespace repository{

std::vector<std::string> cache;

namespace event {
  bool add(const std::string& hash, std::unique_ptr<abstract_transaction::AbstractTransaction> tx){
    if(tx->getType() == abstract_transaction::TransactionType::message){
       /*
        world_state_repository::add(
            hash,
            convert::to_string<
              abstract_transaction::AbstractTransaction
            >(std::move(tx))
        );
        */
    }
  }

  bool update(const std::string& hash, const abstract_transaction::AbstractTransaction& tx){

  }
  bool remove(const std::string& hash){

  }
  bool empty(){

  }
  std::vector<
    std::unique_ptr<abstract_transaction::AbstractTransaction>
  > findAll(){

  }

  std::unique_ptr<abstract_transaction::AbstractTransaction>& find(std::string hash){

  }
};
};