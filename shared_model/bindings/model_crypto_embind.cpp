#include <emscripten/bind.h>

#include "model_crypto.hpp"

using namespace emscripten;
using namespace shared_model::bindings;
using namespace shared_model::crypto;

EMSCRIPTEN_BINDINGS(model_crypto) {
  register_vector<uint8_t>("ByteVector");
  register_vector<std::string>("StringVector");

  class_<Blob>("Blob")
      .constructor<const std::string &>()
      .function("blob", &Blob::blob)
      .function("hex", &Blob::hex)
      .function("size", &Blob::size);

  class_<Signed, base<Blob>>("Signed")
      .constructor<const std::string &>()
      .function("toString", &Signed::toString);

  class_<PublicKey, base<Blob>>("PublicKey")
      .constructor<const std::string &>()
      .function("toString", &PublicKey::toString);

  class_<PrivateKey, base<Blob>>("PrivateKey")
      .constructor<const std::string &>()
      .function("toString", &PrivateKey::toString);

  class_<Keypair>("Keypair")
      .constructor<PublicKey, PrivateKey>()
      .function("publicKey", &Keypair::publicKey)
      .function("privateKey", &Keypair::privateKey);

  /**
   * Top level ModelCrypto class
   **/
  class_<ModelCrypto>("ModelCrypto")
      .constructor<>()
      .function("generateKeypair", &ModelCrypto::generateKeypair)
      .function("fromPrivateKey", &ModelCrypto::fromPrivateKey)
      .function("convertFromExisting", &ModelCrypto::convertFromExisting);
}
