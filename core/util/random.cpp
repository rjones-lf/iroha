#include "random.hpp"

namespace random_service{

    std::string makeRandomHash() {
        std::random_device rd;
        return hash::sha3_256_hex(std::to_string(rd()));
    }

};  // namespace random_service
