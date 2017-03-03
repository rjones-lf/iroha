cmake_minimum_required(VERSION 2.8)

# Base64
ADD_LIBRARY(base64 STATIC base64.cpp)

# Signature
ADD_LIBRARY(signature STATIC signature.cpp)
target_link_libraries(signature
  ed25519
  base64
)

# Hash
ADD_LIBRARY(hash STATIC hash.cpp)
target_link_libraries(hash
  keccak
)
