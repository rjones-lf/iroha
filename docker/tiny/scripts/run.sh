#!/bin/sh
# gemerate single sumeragi.json
${IROHA_HOME}/bin/make_sumeragi -i eth0 -n $(hostname) > ${IROHA_HOME}/config/sumeragi.json

# run iroha
${IROHA_HOME}/bin/iroha-main