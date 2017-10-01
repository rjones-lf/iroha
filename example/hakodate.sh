#!/bin/bash

#------------------------------------------------------------------------
# API Specification from proto files
#
# [ commands.proto ]
# CreateDomain
#   domain_name
# AddPeer
#   address, peer_key
# CreateAsset
#   asset_name, domain_id, precision
# CreateAccount
#   account_name, domain_id, main_pubkey
# SetAccountPermissions
#   account_id, permissions
# SetAccountQuorum
#   account_id, quorum
# AddSignatory
#   account_id, public_key
# RemoveSignatory
#   account_id, public_key
# AddAssetQuantity
#   account_id, assetid, amount
# TransferAsset
#   src_account_id, dest_account_id, asset_id, description, amount
# AppendRole
#   account_id, role_name
# CreateRole
#   role_name, permissions
# GrantPermission
#   account_id, permission_name
# RevokePermission
#   account_id, permission_name
# 
# [ queries.proto ]
# GetAccount
#   account_id
# GetSignatories
#   account_id
# GetAccountTransactions
#   account_id
# GetAccountAssetTransactions
#   account_id, asset_id
# GetAccountAssets
#   account_id, asset_id
# GetAssetInfo
#   asset_id
# GetRoles
# GetRolePermissions
#   role_id
# 
# [ primitive.proto - Permissions ]
# create_assets
# create_accounts
# create_domains
# read_all_accounts
# add_signatory
# remove_signatory
# set_permissions
# set_quorum
# can_transfer
#
# Specification of Identifiers
#
# domain_id : `^[a-z_0-9]{1,32}(\.[a-z_0-9]{1,32}){0,4}$`
# asset_id : `^[a-z_0-9]{1,32}(\.[a-z_0-9]{1,32}){0,4}\/[a-z_0-9]{1,32}$`
# account_id : `^[a-z_0-9]{1,32}\@[a-z_0-9]{1,32}(\.[a-z_0-9]{1,32}){0,4}$`
# quorum : `quorum !=0 and account.quorum <= size(account.signatures)`
# pubkey : `len(pubkey) == 32`
# hash : `len(hash) == 32`
# created_time : `belong to 1 day ago and now`
# amount : `0 < amount and amount <= account.amount` (edited)
#
#------------------------------------------------------------------------

IROHA_NODE=iroha_node_1
IROHA_HOME=/usr/local/iroha
IROHA_CLI=${IROHA_HOME}/bin/iroha-cli
IROHA_SEND="--grpc --json_transaction"
IROHA_RECV="--grpc --json_query"

tx_counter=0
rx_counter=0

CURDIR=$(basename $(pwd))

function send() {
  echo "=== $1 ==="
  read junk

  created_ts=$(./current_millis)
  ((tx_counter+=1))

  cat $(basename $2) | \
    sed -e "s/\"created_ts\".*/\"created_ts\": $created_ts,/" \
        -e "s/\"tx_counter\".*/\"tx_counter\": $tx_counter,/" >tx.json

  docker exec -t ${IROHA_NODE} ${IROHA_CLI} ${IROHA_SEND} ${CURDIR}/tx.json
  echo
}

function recv() {
  echo "=== $1 ==="
  read junk

  created_ts=$(./current_millis)
  ((tx_counter+=1))

  cat $(basename $2) | \
    sed -e "s/\"created_ts\".*/\"created_ts\": $created_ts,/" \
        -e "s/\"tx_counter\".*/\"tx_counter\": $tx_counter,/" >rx.json

  docker exec -t ${IROHA_NODE} ${IROHA_CLI} ${IROHA_RECV} ${CURDIR}/rx.json
  echo
}

## # Generate New Key-pair by iroha-cli
## docker exec -t iroha_node_1 /usr/local/iroha/bin/iroha-cli --new_account --name alice --pass_phrase hell0wor1d

send "CreateDomain"             CreateDomain.json

send "CreateAsset"              CreateAsset.json

recv "GetAccount (alice)"       GetAccount-alice.json
send "CreateAccount (alice)"    CreateAccount-alice.json
recv "GetAccount (alice)"       GetAccount-alice.json

send "CreateAccount (bob)"      CreateAccount-bob.json
recv "GetAccount (bob)"         GetAccount-bob.json

send "AddSignatory (bob)"       AddSignatory-bob.json
recv "GetSignatories (bob)"     GetSignatories.json

send "AddAssetQuantity (alice)" AddAssetQuantity-alice.json
recv "GetAccountAssets (alice)" GetAccountAssets-alice.json

send "AddAssetQuantity (bob)"   AddAssetQuantity-bob.json
recv "GetAccountAssets (bob)"   GetAccountAssets-bob.json

send "TransferAsset"            TransferAsset.json
recv "GetAccountAssets (alice)" GetAccountAssets-alice.json
recv "GetAccountAssets (bob)"   GetAccountAssets-bob.json

recv "GetAccountTransactions (bob)" GetAccountTransactions.json

recv "GetAccountAssetTransactions (alice)" GetAccountAssetTransactions-alice.json

exit 0
