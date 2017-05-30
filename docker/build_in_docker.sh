#!/bin/bash
# for error handling:
set -o pipefail  # trace ERR through pipes
set -o errtrace  # trace ERR through 'time command' and other functions
set -o nounset   ## set -u : exit the script if you try to use an uninitialised variable
set -o errexit   ## set -e : exit the script if any statement returns a non-true return value


if [ -z ${IROHA_HOME} ]; then
    echo "[FATAL] Empty variable IROHA_HOME"
    exit 1
fi


SOURCE=${IROHA_HOME}
BUILD=${SOURCE}/build
TINY=${IROHA_HOME}/docker/tiny
RELEASE=${TINY}/iroha


# run dev container to build iroha
docker run -i \
    -v ${IROHA_HOME}/docker/build:/build \
    -v ${IROHA_HOME}:/opt/iroha \
    -v ${TINY}:/tiny \
    -v ${RELEASE}:/release \
    warchantua/iroha-dev \
    sh << "COMMANDS"
    # everything between COMMANDS will be executed inside a container

    # clean current binaries
    rm -rf /build/* /opt/iroha/external

    # make iroha
    cd /build
    cmake /opt/iroha -DCMAKE_BUILD_TYPE=Release
    make -j 10

    # copy libs used by iroha
    LIBS=$(ldd /build/bin/iroha-main | cut -f 2 | cut -d " " -f 3)
    mkdir -p /release/lib
    cp -H $LIBS /release/lib

    # copy config
    mkdir -p /release/config
    cp /opt/iroha/config/sumeragi.json /release/config/sumeragi.json
    cp /opt/iroha/config/config.json /release/config/config.json

    rsync -avr /build/bin /release
    rsync -avr /tiny/scripts /release
COMMANDS
if [ ! $? ]; then 
    error "can not build iroha; exit code: $?"
fi

# create iroha-docker image
docker build -t hyperledger/iroha-docker ${TINY}

# clean up
rm -rf ${TINY}/iroha

cat << "EOF"
 _______   ______   .__   __.  _______ 
|       \ /  __  \  |  \ |  | |   ____|
|  .--.  |  |  |  | |   \|  | |  |__   
|  |  |  |  |  |  | |  . `  | |   __|  
|  '--'  |  `--'  | |  |\   | |  |____ 
|_______/ \______/  |__| \__| |_______|
                                      
EOF
