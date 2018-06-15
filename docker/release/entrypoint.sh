#!/usr/bin/env bash
echo key=$KEY
echo $PWD
irohad --genesis_block genesis.block --config config.docker --keypair_name $KEY
