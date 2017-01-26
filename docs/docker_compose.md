# Using docker-compose to setup test network of N nodes on a single computer

[Build](../docker/README.md) or pull latest `hyperledger/iroha-docker` container:
```bash
# build: 
${IROHA_HOME}/docker/build.sh

# pull:
docker pull hyperledger/iroha-docker
```



To run network of N nodes you should do:

1. Change line `command: "4"` in `${IROHA_HOME}/docker-compose.yml` (configdiscovery service) to `command: "N"`
2. `docker-compose up` (-d for detached)
3. `docker-compose scale iroha=N`



If for some reason that didn't work, destroy created containers and try again:

```
docker-compose down
```

