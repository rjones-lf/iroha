#include "random.hpp"

namespace random_service {

std::string makeHashByMT19937() {
  static std::mt19937 mt(std::random_device{}());
  return crypto::hash::sha3_256_hex(std::to_string(mt()));
}

};  // namespace random_service
