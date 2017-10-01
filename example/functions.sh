#!/bin/bash

# to print name of the command or query and wait enter to be pushed
function print_and_read() {
  echo "=== $1 ==="
  read
}

# update tx counter and created ts of tx
function update_tx() {
  created_ts=$(./current_millis)
  ((tx_counter+=1))

  < $(basename $1) \
    sed -e "s/\"created_ts\".*/\"created_ts\": $created_ts,/" \
        -e "s/\"tx_counter\".*/\"tx_counter\": $tx_counter,/" >tx.json
}

# update tx counter and created ts of query
function update_query() {
  created_ts=$(./current_millis)
  ((tx_counter+=1))

  < $(basename $1)  \
    sed -e "s/\"created_ts\".*/\"created_ts\": $created_ts,/" \
        -e "s/\"tx_counter\".*/\"tx_counter\": $tx_counter,/" >rx.json
}

# update and send tx
function send() {
  update_tx $1
  $IROHA_CLI --grpc --json_transaction tx.json
}

# update and recv result of the query
function recv() {
  update_query $1
  $IROHA_CLI --grpc --json_query rx.json
}

# print and send command
function run_send() {
  print_and_read $1
  send $2
  echo
}

# print and run query
function run_recv() {
  print_and_read $1
  recv $2
  echo
}