#!/bin/bash
CURDIR="$(cd "$(dirname "$0")"; pwd)"
IROHA_HOME="$(dirname "${CURDIR}")"
PROJECT=iroha${UID}
COMPOSE=${IROHA_HOME}/docker/docker-compose.yml

docker-compose -f ${COMPOSE} -p ${PROJECT} down
