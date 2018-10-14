#!/usr/bin/env bash

set -ex

GENESISBLOCK_PATH=${GENESISBLOCK_PATH:-genesis.block}
CONFIG_PATH=${CONFIG_PATH:-config.docker}
POSTGRES_PORT=${POSTGRES_PORT:-5432}
BLOCKS_PATH=${BLOCKS_PATH:-`cat ${CONFIG_PATH} | jq -r .block_store_path`}
POSTGRES_HOST=${POSTGRES_HOST:-`cat ${CONFIG_PATH} | jq -r .pg_opt | cut -d' ' -f1 | cut -d'=' -f2`}
WAIT_TIMEOUT=${WAIT_TIMEOUT:-10}

# postgres host is always defined
/wait-for-it.sh -h $POSTGRES_HOST -p $POSTGRES_PORT -t ${WAIT_TIMEOUT} -- true

if [ -z "$(ls -A ${BLOCKS_PATH})" ]; then
  # if ledger is empty then init ledger with genesis_block
	irohad --genesis_block ${GENESISBLOCK_PATH} --config ${CONFIG_PATH} --keypair_name ${KEY}
else
  # else continue to run iroha without genesis_block
	irohad --config ${CONFIG_PATH} --keypair_name ${KEY}
fi
