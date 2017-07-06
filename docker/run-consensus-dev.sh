#!/bin/bash

# change dir to script's file
cd "$(dirname "$0")"

docker-compose -f docker-compose-consensus-dev.yml down
export UID
docker-compose -f docker-compose-consensus-dev.yml up -d
docker-compose -f docker-compose-consensus-dev.yml exec node /bin/bash
