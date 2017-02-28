# How to build `iroha` using docker

Clone `iroha.git` on your directory.

```
git clone --recursive https://github.com/hyperledger/iroha.git
cd iroha
```

Build depends on the environment variable `IROHA_HOME` so you need to set it:

`export IROHA_HOME=$(pwd)`

Run build script and wait for completion. 
```
${IROHA_HOME}/docker/build.sh
``` 

 - It builds `hyperledger/iroha-dev` image, which is ubuntu:latest with all dependencies installed.
 - It builds iroha inside that image and copies `iroha.tar` into `${IROHA_HOME}/docker/build/iroha.tar`
 - It creates production-ready docker image with tag `hyperledger/iroha-docker` and copies `iroha.tar` inside it.


# Usage example

In order to use iroha, you need to create `config/sumeragi.json` config file in each node (docker container). 

You have two options:

 1. To use [automatic config discovery](./config-discovery/README.md).
 2. To run iroha containers and setup configs by yourself. To do this, run bash in iroha container:
```bash
# run bash instead of default /run.sh
ID=$(docker run -it -d hyperledger/iroha-docker bash)
# attach to container
docker exec -it $ID bash
# setup config now and then execute /run.sh
```

iroha image includes several scripts:
 - `/configure.sh`, which makes attempt to get config file from `configdiscovery` service.
 - `/run.sh`, which runs iroha (it will be successful only if you properly setup `${IROHA_HOME}/config/sumeragi.json` inside a container) .
 - `/configure-then-run.sh`, which runs `configure`, then `run`. 

 You can use them instead default `CMD`:

```bash
docker run -d --name iroha hyperledger/iroha-docker /configure-then-run.sh
```


#### Good luck!
