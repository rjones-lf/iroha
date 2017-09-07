#!/bin/bash

USER=iroha
USERID=$(id -u $(whoami))

CURDIR="$(cd "$(dirname "$0")"; pwd)"
IROHA_HOME="$(dirname "${CURDIR}")"
PROJECT=iroha${USERID}

export COMPOSE_PROJECT_NAME=${PROJECT}

if [ ! "$(docker ps -q -f name=${PROJECT}_node_1)" ] # node already running
then
  docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml up -d
  docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml exec node \
    adduser --disabled-password --gecos "" ${USER} --uid ${USERID}
  docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml exec node \
    chown -R ${USER}:${USER} /tmp/ccache
  docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml exec \
    --user ${USER} node /bin/bash
  docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml down
else
  docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml exec \
    --user ${USER} node /bin/bash
fi
