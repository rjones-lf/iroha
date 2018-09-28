#!/usr/bin/env bash
sed -ri "s/host=.*( port=.*)/host=${pg-host}\1/" config.sample
KEY=$(echo $KEY | cut -d'-' -f2)
sleep 10
irohad --genesis_block genesis.block --config config.sample --keypair_name node$KEY
