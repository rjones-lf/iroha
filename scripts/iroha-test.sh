#!/bin/bash

while read result name; do
  if [[ "$result" == "o" ]]; then
    echo "=============================="
    echo "=== $name ==="
    echo "=============================="
    docker exec iroha_node_1 /usr/local/iroha/test_bin/$name
  # cat </dev/tty
  fi
done <iroha-test.lst
