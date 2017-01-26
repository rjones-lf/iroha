# Deploying a network on a docker swarm

Using this method, it is possible set up a network of N nodes within a few minutes.

Merges to master are automatically deployed to [hub.docker.com](https://hub.docker.com/r/hyperledger/iroha-docker/) under tag `hyperledger/iroha-docker`.


0. Install docker (at least version 1.12)

1. You should setup docker swarm: 

   - master node: 

     ```docker swarm init --advertise-addr=(insert master node IP here)```

   -  worker nodes: 

     ```docker swarm join --token=(insert token here, printed out from the previous step)```
Then, on master node:

2. Create overlay network: 

   ```docker network create -d overlay iroha-network```

3. Create `configdiscovery` service:

```bash
docker service create \
 --network iroha-network \
 --name configdiscovery \
 warchantua/configdiscovery N # replace N with number of nodes
```

This service does automatic `sumeragi.json` config distribution across your network. 
***WARNING***: if you don't use docker swarm, configdiscovery will not work and you have to manage configs by yourself.

4. Create iroha [global](https://docs.docker.com/engine/reference/commandline/service_create/)[^1] service:

```bash
docker service create \
 --name iroha \
 --mode global \
 --network iroha-network \
 --publish 1204:1204 \
 hyperledger/iroha-docker /configure-then-run.sh
```


[^1]: `--mode global` means that each physical node in swarm (master+workers) will run one container.