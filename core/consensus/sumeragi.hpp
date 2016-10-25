#ifndef CORE_CONSENSUS_SUMERAGI_HPP_
#define CORE_CONSENSUS_SUMERAGI_HPP_

#define COMPARATOR(code) [](auto && l, auto && r) -> bool { return code ; }

#include <vector>
#include <memory>

#include "consensus_event.hpp"

#include "../service/peer_service.hpp"

namespace sumeragi {

    void initializeSumeragi(
        const std::string& myPublicKey,
        std::vector<std::unique_ptr<peer::Node>> peers
    );
    void loop();

    void processTransaction(
        std::unique_ptr<consensus_event::ConsensusEvent> const event
    );
    void panic(const std::unique_ptr<consensus_event::ConsensusEvent>& event);
    void setAwkTimer(const int sleepMillisecs, const std::function<void(void)> action);
    void determineConsensusOrder(/std::vector<double> trustVector*/);
};  // namespace sumeragi

#endif  // CORE_CONSENSUS_SUMERAGI_HPP_
