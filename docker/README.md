# How to build `iroha` using docker

Clone `iroha.git` on your directory.

```
git clone https://github.com/hyperledger/iroha.git
cd iroha
```

Build depends on the environment variable `IROHA_HOME` so you need to set it:

`export IROHA_HOME=$(pwd)`

Run build script and wait for completion. 

```bash
${IROHA_HOME}/docker/build.sh
``` 

Docker image with tag `hyperledger/iroha-docker` will be built.

# How to run

```
$ sudo docker run -d -it --name=iroha -p 50051:50051 hyperledger/iroha-docker
```


# Troubleshooting

To successfully build docker image, you should be able to build it locally: [follow this guilde](../docs/how_to_build.rst) in case of troubles.
