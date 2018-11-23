#!/usr/bin/env bash
if [ -z "$(ls -A /opt/block_store)" ]; then
	irohad --genesis_block genesis.block --config config.sample --keypair_name node
else
	irohad --config config.sample --keypair_name node
fi
