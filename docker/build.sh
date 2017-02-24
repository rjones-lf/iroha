#!/bin/bash

if [ -z ${IROHA_HOME} ]; then
    echo "[FATAL] Empty variable IROHA_HOME"
    exit 1
fi

error(){
	echo "[Error] $1"
	exit 1
}

# build iroha-dev image
if ! docker images hyperledger/iroha-dev | grep -q hyperledger/iroha-dev; then
  docker build --rm=false -t hyperledger/iroha-dev ${IROHA_HOME}/docker/dev 
  if [ ! $? ]; then 
      error "can not build iroha-dev; exit code: $?"
  fi
fi

# run dev container to build iroha
docker run -i \
    -v ${IROHA_HOME}/docker/build:/build \
    -v ${IROHA_HOME}:/opt/iroha \
    hyperledger/iroha-dev \
    sh << COMMANDS
    # everything between COMMANDS will be executed inside a container
    cd /opt/iroha
    /build-iroha.sh || (echo "[-] Can't build iroha" && exit 1)
    /mktar-iroha.sh || (echo "[-] Can't make tarball" && exit 1)
    # at this step we have /tmp/iroha.tar 
    (cp /tmp/iroha.tar /build/iroha.tar || (echo "[-] FAILED!" && exit 1))
COMMANDS
if [ ! $? ]; then 
    error "can not build iroha; exit code: $?"
fi

# build hyperledger/iroha container
docker build --rm=false -t hyperledger/iroha-docker ${IROHA_HOME}/docker/build
if [ ! $? ]; then 
    error "can not build iroha; exit code: $?"
fi
