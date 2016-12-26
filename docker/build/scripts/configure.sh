#!/bin/bash

${IROHA_HOME}/bin/make_sumeragi -i eth0 -n `hostname` -o /tmp/me.json && \
mkdir -p ${IROHA_HOME}/config && \
cat /tmp/me.json | nc configdiscovery 8000 > ${IROHA_HOME}/config/sumeragi.json || \
echo "[FAILED][configure.sh]"
