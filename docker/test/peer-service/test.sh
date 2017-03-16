#!/bin/sh
counter=0

# create a network
docker network create --subnet 10.0.0.0/16 iroha

run_node(){
  # you should supply first argument to run_node
  # $1 should be integer > 0
  index=${1:-1} # first argument or default value=1
  # run node (but don't start iroha-main, start bash instead)
  # because node will be running with old config
  docker run -dit \
    --network=iroha \
    --ip=10.0.1.${index} \
    --name=iroha${index} \
    hyperledger/iroha-docker bash
  # copy config inside image
  docker cp sumeragi${index}.json iroha${index}:/iroha/config/sumeragi.json
  # and then run iroha-main, -d = detached
  docker exec iroha${index} /iroha/bin/iroha-main > /tmp/iroha${index}.log 2>&1 &
}

# run two nodes
run_node 1 # 10.0.1.1
run_node 2 # 10.0.1.2

## Tests start

## Tests end

# clean up
#docker rm -f iroha1
#docker rm -f iroha2
#docker network rm iroha
