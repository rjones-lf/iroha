#include <vector>
#include "data.h"

class Winternitz {
  public:
    Winternitz(Data sk, unsigned int ell);
    ~Winternitz();

    // returns the parameters, l, t, t', n, in a string
    std::string toString();

    // calculates and returns signature given a message digest
    std::vector<Data> sign(Data digest);

    // calculates and returns public key given sk
    Data getPublicKey();

    // Returns the size of the Winternitz Signature object
    long getSize();

    /* --- Static Functions For Verification --- */

    // Verifies if a signature is valid given digest,
    // signature and public key.
    static bool verifySignature(Data digest, std::vector<Data> &sig, Data publicKey, unsigned int ell);

    // calculates the public key that corresponds to
    // the digest, signature and parameter ell.
    static Data calculateVerifiedSig(Data digest, std::vector<Data> &sig, unsigned int ell);

  private:
    unsigned int l; // ell
    unsigned int t;
    unsigned int t_p; // t'
    std::vector<Data> secretKey;

    void generateSecretKeys(Data sk);
    Data generatePublicKey();
    std::vector<Data> calculateSig(std::vector<unsigned int> &b);

    // static functions
    static unsigned int calculateT(unsigned int a, unsigned int b);
    static unsigned int calculateTPrime(unsigned int a, unsigned int b);
    static std::vector<unsigned int> generateB(Data digest, unsigned int t, unsigned int t_p, unsigned int l);
    static std::vector<unsigned int> convertToBase(Data digest, unsigned int t, unsigned int l);
    static std::vector<unsigned int> convertIntegerToBase(CryptoPP::Integer dec, unsigned int t, unsigned int l);
    static void calculateChecksum(std::vector<unsigned int> &b, unsigned int l, unsigned int t_p);

};
