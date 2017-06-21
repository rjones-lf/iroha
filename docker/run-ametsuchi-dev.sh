#!/bin/bash
docker-compose -f docker-compose-ametsuchi-dev.yml down
export UID
docker-compose -f docker-compose-ametsuchi-dev.yml up -d
docker-compose -f docker-compose-ametsuchi-dev.yml exec node /bin/bash
