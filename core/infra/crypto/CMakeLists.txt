
SET(KECCAK_PATH  "${PROJECT_SOURCE_DIR}/core/vendor/KeccakCodePackage")
SET(KECCAK_BIN_PATH  "${PROJECT_SOURCE_DIR}/core/vendor/KeccakCodePackage/bin/.build/generic64/")
SET(ED25519_PATH "${PROJECT_SOURCE_DIR}/core/vendor/ed25519")

# Hash

# Bad solution (T A T)
execute_process(COMMAND "cd ${PROJECT_SOURCE_DIR}/core/infra/crypto; make")

#[[
#SET(CMAKE_CXX_FLAGS "-g -Wall -std=c++1y -fomit-frame-pointer -march=native -mtune=native -m64")
#
# Why not running??
#
#add_definitions(
#  -DKeccakP200_excluded
#  -DKeccakP400_excluded
#  -DKeccakP800_excluded
#)
#
#include_directories(
#  ${KECCAK_PATH}/bin/generic64/libkeccak.a.headers
#  ${ED25519_PATH}/src
#)
#link_directories(
#  ${ED25519_PATH}/lib
#  ${KECCAK_PATH}/bin/generic64
#)
#
# hash
#ADD_LIBRARY(hash STATIC
#  ${KECCAK_PATH}/SnP/KeccakP-1600/Optimized64/KeccakP-1600-opt64.c
#  ${KECCAK_PATH}/Constructions/KeccakSponge.c
#  ${KECCAK_PATH}/Modes/SimpleFIPS202.c
#  hash.cpp
#  ${KECCAK_PATH}/Modes/SimpleFIPS202.c
#)
#target_link_libraries(hash
#  keccak
#)
# ]]

# Base64
ADD_LIBRARY(base64 STATIC
  base64.cpp
)

# Signature
include_directories(${ED25519_PATH}/src)
link_directories(${ED25519_PATH}/lib)

ADD_LIBRARY(signature STATIC
  signature.cpp
)
target_link_libraries(signature
  ed25519
  base64
)