SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(KECCAK_PATH  "${PROJECT_SOURCE_DIR}/core/vendor/KeccakCodePackage")
SET(ED25519_PATH "${PROJECT_SOURCE_DIR}/core/vendor/ed25519")

include_directories(
  ${ED25519_PATH}/src
)

link_directories(
  ${ED25519_PATH}/lib
  ${KECCAK_PATH}/bin/generic64
)

ADD_LIBRARY(validator  STATIC
  #consensus_event_validator.cpp
  transaction_validator.cpp
)

target_link_libraries(validator
  signature
)
