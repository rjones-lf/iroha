#!/bin/bash

# TODO: change warchantua to hyperledger.

if [ -z ${IROHA_HOME} ]; then
    echo "[FATAL] Empty variable IROHA_HOME"
    exit 1
fi

# pull image from docker-hub, or (in case of fail), build new
docker pull warchantua/iroha-dev || \
    docker build -t warchantua/iroha-dev ${IROHA_HOME}/docker/dev 

# run dev container to build iroha
docker run -i --rm \
    -v ${IROHA_HOME}/docker/build:/build \
    -v ${IROHA_HOME}:/opt/iroha \
    warchantua/iroha-dev \
    sh << COMMANDS
    # everything between COMMANDS will be executed inside a container
    cd /opt/iroha
    /build-iroha.sh || (echo "[-] Can't build iroha" && exit 1)
    /mktar-iroha.sh || (echo "[-] Can't make tarball" && exit 1)
    # at this step we have /tmp/iroha.tar 
    (cp /tmp/iroha.tar /build/iroha.tar || \
        (echo "[-] FAILED! Mount /build folder from your host or use iroha/docker/build.sh script!" && exit 1))
COMMANDS

# build warchantua/iroha container
docker build -t warchantua/iroha ${IROHA_HOME}/docker/build
