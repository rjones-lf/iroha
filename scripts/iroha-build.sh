#!/bin/bash

export IROHA_HOME=/opt/iroha

rm -fr ${IROHA_HOME}/external

cd ${IROHA_HOME}/build

cmake ${IROHA_HOME} -DCMAKE_BUILD_TYPE=Release
make -j 10

if [[ $? != 0 ]]; then
  exit 1
fi

LIBS=$(ldd ${IROHA_HOME}/build/bin/iroha-main | cut -f 2 | cut -d " " -f 3)
rm -fr ${IROHA_HOME}/build/iroha
mkdir -p ${IROHA_HOME}/build/iroha/lib
cp -H $LIBS ${IROHA_HOME}/build/iroha/lib

mkdir -p ${IROHA_HOME}/build/iroha/config
cp ${IROHA_HOME}/config/sumeragi.json ${IROHA_HOME}/build/iroha/config/sumeragi.json
cp ${IROHA_HOME}/config/config.json   ${IROHA_HOME}/build/iroha/config/config.json

rsync -avr ${IROHA_HOME}/build/bin      ${IROHA_HOME}/build/iroha
rsync -avr ${IROHA_HOME}/build/test_bin ${IROHA_HOME}/build/iroha
