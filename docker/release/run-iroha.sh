#!/usr/bin/env bash

set -xe

GENESISBLOCK_PATH=${GENESISBLOCK_PATH:-genesis.block}
CONFIG_PATH=${CONFIG_PATH:-config.docker}
POSTGRES_PORT=${POSTGRES_PORT:-5432}

pgHostAutodetect=`cat ${CONFIG_PATH} | jq -r .pg_opt | grep -Eo "host=(.*?)\s" | cut -d'=' -f2 | xargs`
blocksPathAutodetect=`cat ${CONFIG_PATH} | jq -r .block_store_path`
BLOCKS_PATH=${BLOCKS_PATH:-${blocksPathAutodetect:-/tmp/block_store}}
POSTGRES_HOST=${POSTGRES_HOST:-${pgHostAutodetect:-localhost}}

# postgres host is always defined
/wait-for-it.sh -h ${POSTGRES_HOST} -p ${POSTGRES_PORT} -t 30 -- true

if [ -z "$(ls -A ${BLOCKS_PATH})" ]; then
  # if ledger is empty then init ledger with genesis_block
	irohad --genesis_block ${GENESISBLOCK_PATH} --config ${CONFIG_PATH} --keypair_name ${KEY}
else
  # else continue to run iroha without genesis_block
	irohad --config ${CONFIG_PATH} --keypair_name ${KEY}
fi
