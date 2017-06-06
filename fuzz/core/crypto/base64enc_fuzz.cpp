#include "stdint.h"
#include "stddef.h"

#include <vector>
#include <crypto/base64.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  const std::vector<unsigned char> s(data, data + size);
  base64::encode(s);
  return 0;
}
