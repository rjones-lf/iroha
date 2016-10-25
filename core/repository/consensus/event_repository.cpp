#include <string>
#include <memory>
#include "event_repository.hpp"

#include <algorithm>

namespace repository{
    namespace event {

        std::vector<
        std::unique_ptr<consensus_event::ConsensusEvent>
        > consensusEvents;

        bool add(const std::string &hash, std::unique_ptr<consensus_event::ConsensusEvent> event) {
            consensusEvents.push_back(std::move(event));
            return false;
        }

        bool update(const std::string &hash, const consensus_event::ConsensusEvent &consensusEvent) {

        }

        bool remove(const std::string &hash) {
            consensusEvents.erase(
                    std::remove_if(
                            consensusEvents.begin(),
                            consensusEvents.end(),
                            [hash](auto&& event) -> bool {
                                return event->merkleRoot == hash;
                            }
                    ),
                    consensusEvents.end()
            );
        }

        bool empty() {
            return consensusEvents.empty();
        }

        std::vector<std::unique_ptr<consensus_event::ConsensusEvent>>&& findAll(){
            return std::move(consensusEvents);
        }

        std::unique_ptr<
        consensus_event::ConsensusEvent
        >&& findNext() {
            auto res = std::move(consensusEvents.front());
            consensusEvents.pop_back();
            return std::move(res);
        }

        std::unique_ptr<consensus_event::ConsensusEvent> find(std::string hash) {
            for(auto&& event : consensusEvents){
                if( event->merkleRoot == hash) return std::move(event);
            }
            return std::unique_ptr<consensus_event::ConsensusEvent>(nullptr);
        }
    };
};
