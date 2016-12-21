#!/bin/bash

# Run this script inside 'iroha-dev' docker container to build iroha and its dependencies!

error(){
    echo "[Error] $1"
    exit 1
}


if [ -z ${IROHA_HOME} ]; then
	error "Empty variable IROHA_HOME"
fi


# build dependencies
(cd $IROHA_HOME/core/vendor/leveldb && make -j4)    || error "Can't build leveldb submodule"
(cd $IROHA_HOME/core/vendor/ed25519 && make -j4)    || error "Can't build ed25519 submodule"
(cd $IROHA_HOME/core/vendor/KeccakCodePackage && \
    make -j4 && \
    make -j4 generic64/libkeccak.a)                 || error "Can't build KeccakCodePackage submodule"
(cd $IROHA_HOME/core/infra/crypto && make -j4)      || error "Can't build crypto submodule"

# build iroha (important: build using single thread!)
(cd $IROHA_HOME                                                                          && \
    (mkdir $IROHA_HOME/build || echo "[+] build directory already exists, rebuilding")   && \
    cd $IROHA_HOME/build                                                                 && \
    cmake ..                                                                             && \
    make )                      || error "Can't build iroha"