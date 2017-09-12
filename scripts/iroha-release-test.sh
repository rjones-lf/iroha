#!/bin/bash

IROHA_HOME=/usr/local/iroha
IROHA_REL=release/iroha

rm -fr ${IROHA_HOME}/docker/${IROHA_REL}

LIBS=$(ldd ${IROHA_HOME}/build/bin/irohad | cut -f 2 | cut -d " " -f 3)
mkdir -p ${IROHA_HOME}/docker/${IROHA_REL}/lib
cp -H ${LIBS} ${IROHA_HOME}/docker/${IROHA_REL}/lib

cp -r ${IROHA_HOME}/build/bin ${IROHA_HOME}/docker/${IROHA_REL}
cp -r ${IROHA_HOME}/config ${IROHA_HOME}/docker/${IROHA_REL}
## Too big binaries
cp -r ${IROHA_HOME}/build/test_bin ${IROHA_HOME}/docker/${IROHA_REL}

exit 0
