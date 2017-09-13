#!/bin/bash

CURDIR="$(cd "$(dirname "$0")"; pwd)"
IROHA_HOME="$(dirname "${CURDIR}")"

export COMPOSE_FILE=${IROHA_HOME}/docker/docker-compose-swarm.yml

# create empty file for list of peers
> ${IROHA_HOME}/peers.list
# append each peer to the file
COUNT=$(docker-machine ls -f "{{.Name}}" | wc -l)
((COUNT--))
for i in $(seq 0 ${COUNT})
do
    echo "iroha${i}_node_1:10001" | cat >> ${IROHA_HOME}/peers.list
done

# create overlay network
eval $(docker-machine env $(docker-machine ls -f "{{.Name}}" | head -1))
docker network create --driver overlay --attachable iroha_network

# set up peers
i=0
while read -r node
do
    # switch docker daemon
    eval $(docker-machine env ${node})
    export COMPOSE_PROJECT_NAME=iroha${i}

    # create services
    docker-compose up -d

    # wait for postgres start
    until [ "$(docker inspect -f {{.State.Running}} ${COMPOSE_PROJECT_NAME}_postgres_1)" == "true" ]
    do
        sleep 0.1;
    done

    # wait for postgres accepting connections
    until docker exec ${COMPOSE_PROJECT_NAME}_postgres_1 pg_isready
    do
        sleep 0.1;
    done

    # wait for redis start
    until [ "$(docker inspect -f {{.State.Running}} ${COMPOSE_PROJECT_NAME}_redis_1)" == "true" ]
    do
        sleep 0.1;
    done

    # wait for redis accepting connections
    until docker exec ${COMPOSE_PROJECT_NAME}_redis_1 redis-cli ping
    do
        sleep 0.1;
    done

    # generate config
    # TODO replace with environment variables
    echo "{
      \"block_store_path\" : \"/tmp/block_store/\",
      \"torii_port\" : 50051,
      \"pg_opt\" : \"host=${COMPOSE_PROJECT_NAME}_postgres_1 port=5432 user=iroha password=helloworld\",
      \"redis_host\" : \"${COMPOSE_PROJECT_NAME}_redis_1\",
      \"redis_port\" : 6379
    }" > ${IROHA_HOME}/iroha.conf

    # copy list of peers
    docker cp ${IROHA_HOME}/peers.list ${COMPOSE_PROJECT_NAME}_node_1:/
    # copy config
    docker cp ${IROHA_HOME}/iroha.conf ${COMPOSE_PROJECT_NAME}_node_1:/
    # generate genesis block based on list of peers
    docker exec ${COMPOSE_PROJECT_NAME}_node_1 /opt/iroha/iroha-cli --genesis_block --peers_address /peers.list
    # run irohad with output redirection to /iroha.log
    docker exec -d ${COMPOSE_PROJECT_NAME}_node_1 bash -c "/opt/iroha/irohad --config /iroha.conf --genesis_block /genesis.block --peer_number ${i} > /iroha.log"
    ((i++))
done < <(docker-machine ls -f "{{.Name}}")

# cleanup
rm ${IROHA_HOME}/peers.list ${IROHA_HOME}/iroha.conf
