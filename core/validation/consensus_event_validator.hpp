#ifndef CORE_VALIDATION_CONSENSUSEVENTVALIDATOR_HPP_
#define CORE_VALIDATION_CONSENSUSEVENTVALIDATOR_HPP_

#include "../consensus/consensus_event.hpp"
#include <type_traits>

namespace consensus_event_validator {

  template <
    typename T,
    std::enable_if_t<
            std::is_base_of<abstract_transaction::AbstractTransaction, T>::value, std::nullptr_t
    > = nullptr
  >
  using Abs = T;

  bool isValid(const consensus_event::ConsensusEvent<Abs>& event) {

  };

  bool isValid(const std::string& event) {
     return true;
  };

    bool signaturesAreValid(const consensus_event::ConsensusEvent<Abs>& event) {
      for (auto sig : event.signatures) {
          if (!consensus_event_validator::isValid(sig)) {
              return false;
          }
      }
  };

};  // namespace consensus_event_validator

#endif  // CORE_VALIDATION_CONSENSUSEVENTVALIDATOR_HPP_
