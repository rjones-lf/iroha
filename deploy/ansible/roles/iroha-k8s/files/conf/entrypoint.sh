#!/usr/bin/env bash
# Wait for Postgres container
sleep 10
irohad --genesis_block genesis.block --config config.sample --keypair_name node

