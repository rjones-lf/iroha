#!/bin/bash
docker rm $(docker ps -a -q -f status=exited)
docker rmi -f hyperledger/iroha-docker
rm -f $IROHA_HOME/docker/build/iroha.tar