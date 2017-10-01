#!/bin/bash

source functions.sh

if [[ -z "${IROHA_CLI}" ]];
    then IROHA_CLI=/opt/iroha/build/bin/iroha-cli
fi

send CreateDomain.json
send CreateAsset.json
send CreateAccount-alice.json
send CreateAccount-bob.json
send AddSignatory-bob.json
send AddAssetQuantity-alice.json
send AddAssetQuantity-bob.json
send TransferAsset.json