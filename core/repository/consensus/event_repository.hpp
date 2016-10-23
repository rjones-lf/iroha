#ifndef CORE_REPOSITORY_UNCONFIRMEDTRANSACTIONREPOSITORY_HPP_
#define CORE_REPOSITORY_UNCONFIRMEDTRANSACTIONREPOSITORY_HPP_

#include <string>
#include <memory>
#include "../../model/transactions/abstract_transaction.hpp"
#include "../../consensus/consensus_event.hpp"

namespace repository {
namespace event {
  bool add(const std::string& hash, std::unique_ptr<consensus_event::ConsensusEvent> consensusEvent);
  bool update(std::string hash, const consensus_event::ConsensusEvent& consensusEvent);
  bool remove(std::string hash);
  bool empty();

  std::vector<
    std::unique_ptr<consensus_event::ConsensusEvent>
  > findAll();

  std::unique_ptr<consensus_event::ConsensusEvent>& find(std::string hash);
};  // namespace unconfirmed_transaction_repository
};
#endif  // CORE_REPOSITORY_UNCONFIRMEDTRANSACTIONREPOSITORY_HPP_
