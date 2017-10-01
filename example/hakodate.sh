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

source functions.sh

#IROHA_NODE=iroha_node_1
#IROHA_HOME=/opt/iroha/build/bin
if [[ -z "${IROHA_CLI}" ]];
    then IROHA_CLI=/opt/iroha/build/bin/iroha-cli
fi
#IROHA_CLI=/opt/iroha/build/bin/iroha-cli
#IROHA_SEND="--grpc --json_transaction"
#IROHA_RECV="--grpc --json_query"

tx_counter=0
#CURDIR=$(basename $(pwd))

## # Generate New Key-pair by iroha-cli
## docker exec -t iroha_node_1 /usr/local/iroha/bin/iroha-cli --new_account --name alice --pass_phrase hell0wor1d

run_send "CreateDomain"             CreateDomain.json

run_send "CreateAsset"              CreateAsset.json

run_recv "GetAccount (alice)"       GetAccount-alice.json
run_send "CreateAccount (alice)"    CreateAccount-alice.json
run_recv "GetAccount (alice)"       GetAccount-alice.json

run_send "CreateAccount (bob)"      CreateAccount-bob.json
run_recv "GetAccount (bob)"         GetAccount-bob.json

run_send "AddSignatory (bob)"       AddSignatory-bob.json
run_recv "GetSignatories (bob)"     GetSignatories.json

run_send "AddAssetQuantity (alice)" AddAssetQuantity-alice.json
run_recv "GetAccountAssets (alice)" GetAccountAssets-alice.json

run_send "AddAssetQuantity (bob)"   AddAssetQuantity-bob.json
run_recv "GetAccountAssets (bob)"   GetAccountAssets-bob.json

run_send "TransferAsset"            TransferAsset.json
run_recv "GetAccountAssets (alice)" GetAccountAssets-alice.json
run_recv "GetAccountAssets (bob)"   GetAccountAssets-bob.json

run_recv "GetAccountTransactions (bob)" GetAccountTransactions.json

run_recv "GetAccountAssetTransactions (alice)" GetAccountAssetTransactions-alice.json

exit 0
