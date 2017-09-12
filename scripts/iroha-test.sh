#!/bin/bash

TEST=$(docker exec -t iroha_node_1 /bin/ls /usr/local/iroha/test_bin | sed 's/\r//')

NAME=$(for i in ${TEST}; do
  echo $i
done | sort
)

for i in ${NAME}; do
  echo "=== $i ==="
  docker exec -t iroha_node_1 /usr/local/iroha/test_bin/$i
done
