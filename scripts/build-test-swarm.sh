#!/bin/sh

if [ -z "$1" ] || [ "$1" -le "0" ]; then
    echo "Usage: $0 <number_of_peers>"
    exit -1
fi
PEERS_NUM="$1"


if ! docker ps >/dev/null 2>&1; then
    echo "Dockerd not running, fix that please"
    exit 2
fi

if ! docker-machine >/dev/null 2>&1; then
    echo "docker-machine not found"
    exit 3
fi

IROHA_HOME="`dirname ${BASH_SOURCE[0]}`/.."
IMAGE="$IROHA_HOME/build/iroha-dev.tar"
PREFIX=peer

docker-machine create ${PERFIX}0

for i in `seq 0 $PEERS_NUM`; do
    docker-machine create "${PERFIX}$i";
    
    if [ -f "$IMAGE" ]; then
        docker-machine ssh "${PERFIX}$i" "docker load -i $IMAGE";
    fi
    
    if [ "$i" -eq 0 ]; then
        INVITE=$(docker-machine ssh ${PERFIX}0 'docker swarm init --advertise-addr=`docker-machine env ${PERFIX}0 | egrep -o "[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}"`' \
                     | egrep -o 'docker\sswarm\sjoin\s.*$')
    else
        docker-machine ssh "${PERFIX}$i" "$INVITE";
        # only for tests, consider ~3 manager nodes in production
        docker-machine ssh ${PERFIX}0 "docker node promote ${PERFIX}$i";
    fi

done

`dirname $(realpath ${BASH_SOURCE[0]})`/swarm-deploy.sh