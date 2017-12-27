#include "stdint.h"
#include "stddef.h"

#include <string>
#include <vector>
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::string s((const char*)data, size);
  hash::sha3_256_hex(s);
  return 0;
}
