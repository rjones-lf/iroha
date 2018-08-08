#!/bin/bash

export PG_CONTAINER_NAME="postgres"
PG_HOST_DATA="/tmp/pgdata"
export PG_PORT="5432"

docker stop ${PG_CONTAINER_NAME}; docker rm -f ${PG_CONTAINER_NAME}
rm -rf ${PG_HOST_DATA}

echo "postgres container is successfully stopped and removed from your OS"
