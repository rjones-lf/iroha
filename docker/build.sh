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


mkdir -p ${BUILD} || echo "${BUILD} folder exists"
cd ${BUILD}
cmake ${IROHA_HOME} -DCMAKE_BUILD_TYPE=Release
make -j 10


# copy libs used by iroha
LIBS=$(ldd $BUILD/bin/iroha-main | cut -f 2 | cut -d " " -f 3)
mkdir -p $RELEASE/lib
cp -H $LIBS $RELEASE/lib

# copy config
mkdir -p ${RELEASE}/config
cp ${IROHA_HOME}/config/sumeragi.json ${RELEASE}/config/sumeragi.json
cp ${IROHA_HOME}/config/config.json ${RELEASE}/config/config.json


rsync -avr ${BUILD}/bin ${RELEASE}
rsync -avr ${TINY}/scripts ${RELEASE}

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
