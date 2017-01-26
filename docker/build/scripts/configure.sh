#!/bin/bash

${IROHA_HOME}/bin/make_sumeragi -i eth0 -n `hostname` -o /tmp/me.json && \
mkdir -p ${IROHA_HOME}/config && \
(nc configdiscovery 8000 < /tmp/me.json) > ${IROHA_HOME}/config/sumeragi.json || \
echo "[FAILED][configure.sh]"
