#!/bin/bash

USERID=$(id -u $(whoami))


if [ "$USERID" == "0" ]; then
    USER=root
else
    USER=iroha
fi

CURDIR="$(cd "$(dirname "$0")"; pwd)"
IROHA_HOME="$(dirname "${CURDIR}")"
PROJECT=iroha${USERID}

export COMPOSE_PROJECT_NAME=${PROJECT}

if [ ! "$(docker ps -q -f name=${PROJECT}_node_1)" ] # node already running
then
  docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml up -d

  if [ "$USERID" != "0" ]; then
      docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml exec \
        --user root node adduser --disabled-password --gecos "" ${USER} --uid ${USERID}
      docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml exec \
        --user root node chown -R ${USER}:${USER} /tmp/ccache
  fi

  docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml exec \
    --user ${USER} node /bin/bash
  docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml down
else
  docker-compose -f ${IROHA_HOME}/docker/docker-compose.yml exec \
    node /bin/bash
fi
