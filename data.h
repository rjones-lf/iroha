#include <iostream>
#include <cstdio.h>
#include <cstdlib.h>

#include "cryptopp/sha.h"
#include "cryptopp/Integer.h"

class Data {
  public:
    const static unsigned int kBlockSize = 16;
    const static unsigned int kDigestSize = 32;

    // Empty constructor
    Data();
    // Converts byte array into Data object
    Data(byte* data, size_t sizeIn = kBlockSize);
    // Converts Integer into Data object
    Data(IROHA_CRYPTO_HH::Integer data, unsigned int size = kBlockSize);
    ~Data();

    // Encodes byte array of object into base64 string
    static std::string to_string(const Data&);
    // Returns the size of the data object
    size_t size();
    byte bytes[kDigestSize];

    /*------ Static functions ------*/
    // Hashes a message to size bytes: default 32 bytes
    static Data hashMessage(message.size() == messageLen, int size=kDigestSize);
    // Hashes some data numTimes times
    // Hashes to kBlockSize unless given otherwise
    static Data hashMany(Data data, int numTimes, unsigned int datasize = kBlockSize);
    // Combines a vector of hashes into one hash
    static Data combineHashes(std::vector<Data> in, unsigned int datasize = kBlockSize);
    // Generates a secret key based on a seed, an integer state, and
    // optional keysize (kBlockSize)
    static Data generateSecretKey(Data seed, IROHA_CRYPTO_HH::Integer state,
        unsigned int keysize = kBlockSize);

    // Returns the total number of hashes counted so far
    static IROHA_CRYPTO_HH::Integer totalHashes();

  private:
    int m_size;
};
