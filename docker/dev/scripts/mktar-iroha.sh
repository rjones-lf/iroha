#!/bin/bash

if [ -z ${IROHA_RELEASE} ]; then
	echo "[FATAL][mktar-iroha.sh] Empty variable IROHA_RELEASE"
	exit 1
fi

cd ${IROHA_RELEASE}
rsync -av ${IROHA_BUILD}/bin $IROHA_RELEASE && \
rsync -av ${IROHA_BUILD}/lib $IROHA_RELEASE && \
rsync -av ${IROHA_BUILD}/my_test_bin $IROHA_RELEASE && \
rsync -av ${IROHA_BUILD}/test_bin $IROHA_RELEASE && \
rsync -av ${IROHA_HOME}/core/vendor/ed25519/lib $IROHA_RELEASE

tar cvf /tmp/iroha.tar \
	/usr/lib/libproto* \
	/usr/local

exit 0
