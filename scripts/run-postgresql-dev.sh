#!/bin/bash

export PG_CONTAINER_NAME="postgres"
PG_IMAGE_NAME="postgres:9.5"
PG_HOST_DATA="/tmp/pgdata"
PG_CONTAINER_DATA="/var/lib/postgresql/data"
PG_USER="iroha"
PG_PASS="helloworld"
export PG_PORT="5432"

export G_ID=$(id -g)
export U_ID=$(id -u)

mkdir -p ${PG_HOST_DATA}
docker run -dt --user "${U_ID}:${G_ID}" \
        --name ${PG_CONTAINER_NAME} \
        -p ${PG_PORT} \
        -v ${PG_HOST_DATA}:${PG_CONTAINER_DATA} \
        -e POSTGRES_USER=${PG_USER} -e POSTGRES_PASSWORD=${PG_PASS} \
        ${PG_IMAGE_NAME}

export PG_ADDR=`docker inspect --format '{''{ .NetworkSettings.IPAddress }''}' ${PG_CONTAINER_NAME}`
echo "postgres is now available at ${PG_ADDR}:${PG_PORT}"
