#include <sstream>
#include <iostream>
#include <vector>

#include "merkle.h"
using IROHA_CRYPTO_HPP::Integer;

#define BLOCKSIZE 16
#define DIGESTSIZE 32

using namespace std;

/*
 * Constructor requires secret key, depth of tree, parameter ell,
 * and whether or not the leaves at the bottom take digests of size
 * 32 bytes or 16 bytes.
 */
Merkle::Merkle(Data sk, unsigned int depthIn, unsigned int digestsizeIn,
    unsigned int ellIn) {
  init(sk, depthIn, digestsizeIn, ellIn);
}

/*
 * Empty constructor so that it is easy to do things like
 * make vectors of trees.
 */
Merkle::Merkle() {
  initialized = false;
  buildingState = 1;
}

/*
 * Initializes tree. Want a zero-argument constructor so that it's
 * easy to make vectors of Merkle trees, so later init can be called
 * to initialize all of the object's values.
 */
void Merkle::init(Data sk, unsigned int depthIn, unsigned int digestsizeIn,
    unsigned int ellIn) {
  ell = ellIn;
  secretKey = sk;
  depth = depthIn;
  digestsize = digestsizeIn;
  buildingState = IROHA_CRYPTO_HPP::Integer::Power2(depth+1) - 1;
  msg = 0;
  initialized = true;
}

/*
 * Returns the important parameters of the tree in string form:
 * depth, whether or not it is a bottom tree, winternitz parameter ell,
 * number of messages already signed, and the public key.
 */
string Merkle::toString() {
  stringstream ss;
  ss << "Merkle signature tree: depth = " << depth << endl;
  ss << "\tdigest size = " << digestsize << endl;
  ss << "\tell = " << ell << endl;
  ss << "\tmessages signed = " << msg << endl;
  ss << "\tpk = " << tree[depth][0].toString() << endl;
  return ss.str();
}

/*** Tree building ***/

/*
 * Returns whether or not init has been called.
 */
bool Merkle::isInitialized() {
  return initialized;
}

/*
 * Returns whether or not the tree has finished building.
 * The variable buildingState counts down from the number of nodes
 * needed in the tree. When it reaches 0, all nodes have been
 * created.
 */
bool Merkle::isCompleted() {
  return (buildingState == 0);
}

/*
 * Calls the operation method until the tree has been built.
 */
void Merkle::buildTree() {
  while(buildingState > 0) {
    operation();
  }
}

/*
 * Does one "operation" to build new tree.
 * An operation makes one node, either a leaf node or a normal
 * node.
 */
void Merkle::operation(){
  if (buildingState <= 0) return;
  if (needLeaf()) {
    makeLeaf();
  } else {
    combine();
  }
  buildingState--;
  if (buildingState <= 0) msg = 0;
}

/*
 * Determines if the next node to create is a leaf node
 * using the stack, s, of nodes.
 */
bool Merkle::needLeaf(){
  if (s.empty()) return true;
  Node x = s.top();
  s.pop();
  if (s.empty()){
    s.push(x);
    return true;
  }
  Node y = s.top();
  s.push(x);
  return (x.height != y.height);
}

/*
 * Makes a new leaf to put in the leaves of the tree.
 * Otherwise, it's just pushed onto the stack for continued calculation
 */
void Merkle::makeLeaf() {
  Data sk = Data::generateSecretKey(secretKey, msg, digestsize);
  msg++;
  Winternitz sig(sk, ell);
  Node leaf;
  leaf.data = sig.getPublicKey();
  leaf.height = 0;
  s.push(leaf);
  winter.push_back(sig); // add leaf to winter
  newEntry(leaf);
}

/*
 * Combines top two entries in stack -- assumes they are at correct height.
 * Makes a new node at the height + 1
 */
void Merkle::combine(){
  vector<Data> vec;
  vec.push_back(s.top().data); s.pop();
  unsigned int height = s.top().height;
  vec.push_back(s.top().data); s.pop();
  Data combine = Data::combineHashes(vec, DIGESTSIZE);
  Node c = {combine, height+1};
  newEntry(c);
  s.push(c);
}

/*
 * Enters a new node into the next spot in the tree at the corresponding
 * height
 * If height not there, adds enough to get there :)
 */
void Merkle::newEntry(Node entry) {
  while (tree.size() < entry.height + 1) {
    tree.push_back(vector<Data>() );
  }
  tree[entry.height].push_back(entry.data);
}


/*** Signing and Verifying ***/

/*
 * Signs a message digest using the next leaf node in the tree, if
 * one is available. Does not do error handling.
 */
Merkle::Signature Merkle::sign(Data digest) {
  // This is just a bitshift -- O(1), no need to store the value
  if (msg >= (1 << depth)) {
    messagesException e;
    throw e;
  }
  long msgNum = getNextMsg();
  Signature merkSig;
  merkSig.wint = winter[msgNum].sign(digest);
  merkSig.auth = getAuthPath(msgNum);
  merkSig.msgNum = msgNum;
  return merkSig;
}

/*
 * In the subclass, this is a critical region.
 */
long Merkle::getNextMsg() {
  msg++;
  return msg.ConvertToLong() - 1;
}

/*
 * Finishes creating the tree if necessary (the pk is the last node created)
 * and returns the root node.
 */
Data Merkle::getPublicKey(){
  if (buildingState > 0) buildTree();
  return tree[depth][0];
}

/*
 * Calculates and returns the size of the Merkle Tree object in bytes
 */
IROHA_CRYPTO_HPP::Integer Merkle::getSize() {
  IROHA_CRYPTO_HPP::Integer size = sizeof(unsigned int) * 3;
  for (size_t i = 0; i < tree.size(); i++){
    for (size_t j = 0; j < tree[i].size(); j++) {
      size+= tree[i][j].size();
    }
  }
  for (size_t i = 0; i < winter.size(); i++) {
    size += winter[i].getSize();
  }
  size += s.size();
  size += sizeof(buildingState);
  size += secretKey.size();
  size += sizeof(msg);
  size += sizeof(initialized);
  return size;
}

/*
 * Verifies if a signature is valid given the message digest, signature,
 * real public key, and winternitz parameter ell.
 */
bool Merkle::verifySignature(Data digest, Merkle::Signature sig, Data publicKey, unsigned int ell) {
  Data pk = calculatePublicKey(digest, sig, ell);
  return (0 == memcmp(pk.bytes, publicKey.bytes, DIGESTSIZE));
}

/*
 * Calculates what the public key should be given a message digest,
 * signature, and Winternitz parameter ell.
 */
Data Merkle::calculatePublicKey(Data digest, Merkle::Signature sig, unsigned int ell) {
  size_t depth = sig.auth.size();
  Integer state = Integer::Power2(depth+1) - 1 - sig.msgNum;
  Data leaf = Winternitz::calculateVerifiedSig(digest, sig.wint, ell);
  //hangs here ^^
  for (size_t i = 0; i < sig.auth.size(); i++) {
    vector<Data> v;
    if (state % 2) { // Odd
      v.push_back(sig.auth[i]);
      v.push_back(leaf);
    } else {  // even
      v.push_back(leaf);
      v.push_back(sig.auth[i]);
    }
    state /= 2;
    leaf = Data::combineHashes(v, DIGESTSIZE);
  }
  return leaf;
}

/*
 * Goes up the tree and creates an authorization path for the stored
 * message number msg.
 */
vector<Data> Merkle::getAuthPath(long msgNum) {
  vector<Data> auth;
  for (unsigned int i = 0; i < depth; i++) {
    if (msgNum % 2) { // odd
      auth.push_back(tree[i][msgNum - 1]);
    } else { // even
      auth.push_back(tree[i][msgNum + 1]);
    }
    msgNum /= 2;
  }
  return auth;
}
