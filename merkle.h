#include "winternitz.h"
#include <stack>

class Merkle {
  public:
    struct Signature {
      std::vector<Data> wint; 
      std::vector<Data> auth; 
      IROHA_CRYPTO_HH::Integer msgNum; 
    };

    // need secret key, depth, size of digest we want to sign, and winternitz sec param
    Merkle(Data sk, unsigned int depth, unsigned int digestsize, unsigned int ell);

    // Empty constructor so that it is easy to create vectors of trees
    Merkle();

    // Initializes the Merkle tree if empty constructor was used.
    void init(Data sk, unsigned int sizeIn, unsigned int digestsize, unsigned int ell);

    // Nothing to destroy.
    ~Merkle() {}

    // Returns important parameters of the tree in string form:
    // deph, Winternitz Parameter ell, number of messages
    // signed, and the public key.
    std::string toString();

// --- Tree building --- //
    // Returns whether or not the tree has been initialized. If not, use init.
    bool isInitialized();

    // Returns whether or not the tree has been completely built.
    bool isCompleted();

    // Builds the entire Merkle tree, until completed.
    void buildTree();

    // Creates one node on the Merkle tree.
    void operation();

// --- Signing and Verifying --- //
    // Signs a message digest using the tree. Assumes tree isCompleted.
    Signature sign(Data digest);

    // Returns the public key -- completes building the tree if necessary
    Data getPublicKey();

    // Returns the size the object is taking up in bytes.
    CryptoPP::Integer getSize();

    /* --- Static Functions For Verification --- */
    // Verifies if a given signature is valid given the message digest,
    // Merkle signature, public key, and Winternitz parameter ell.
    static bool verifySignature(Data digest, Merkle::Signature sig, Data publicKey, unsigned int ell);

    // Calculates what the public key should be based on a digest and given
    // signature (and Winternitz parameter ell).
    static Data calculatePublicKey(Data digest, Merkle::Signature sig, unsigned int ell);

  private:
    // A node in the Merkle tree
    struct Node {
      Data data;
      unsigned int height;
    };
    // Depth of the tree
    unsigned int depth;
    // Tree is a vector of vectors of data
    std::vector<std::vector<Data> > tree;
    // Keep track of the winternitz signatures of a completed tree so we
    // don't have to recalculate secret keys.
    std::vector<Winternitz> winter;

    // For building the tree.
    std::stack<Node> s;
    // Keeps track of the state of the tree as it is being built.
    IROHA_CRYPTO_HH::Integer buildingState;

    // The secret key for the whole tree.
    Data secretKey;
    // Winternitz parameter for the leaves.
    unsigned int ell;
    // Message number. Hopefully it won't go above 2^64 messages, but just in
    // case we will use the Integer class
    IROHA_CRYPTO_HH::Integer msg;
    // Size of the digest we want to sign: 16 or 32 bytes
    unsigned int digestsize;
    // Whether or not it is initialized.
    bool initialized;

    // Adds a new node to the tree at the correct level
    void newEntry(Node entry);
    // Determines if the next node to make should be a leaf node
    bool needLeaf();
    // Makes the next leaf node
    void makeLeaf();
    // combines two nodes together to make a new node
    void combine();

    // Returns the authorization path for the current message
    std::vector<Data> getAuthPath(long msgNum);

    // Returns the current message number and increments it
    virtual long getNextMsg();
};

class messagesException: public std::exception {
  public:
    virtual const char* what() const throw() {
      return "Trying to sign too many messages.";
    }
};
