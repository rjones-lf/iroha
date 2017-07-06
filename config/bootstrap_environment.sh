#!/bin/sh

# this is temporary solution
export IROHA_PEER_PUBKEY_PATH=/home/bogdan/iroha/testbase.pub
export IROHA_PEER_PRIVKEY_PATH=/home/bogdan/iroha/testbase.priv
export IROHA_POSTGRES_HOST=localhost
export IROHA_POSTGRES_PORT=5432
export IROHA_POSTGRES_USER=kek
export IROHA_POSTGRES_PASSWORD=helloworld
export IROHA_REDIS_HOST=localhost
export IROHA_REDIS_PORT=6379
export IROHA_KAFKA_HOST=localhost
export IROHA_KAFKA_PORT=2181
export IROHA_PATH=/home/bogdan/iroha

# debug, info, warning, error, critical, debug, trace, off
export IROHA_VERBOSITY=debug

