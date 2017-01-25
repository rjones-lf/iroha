# How to build `iroha`

#### 1
Ensure, you have `IROHA_HOME` variable setted:
```bash
if [ -z ${IROHA_HOME} ]; then
    echo "[FATAL] Empty variable IROHA_HOME"
    exit 1
fi
```

#### 2
To build `iroha` you need a lot of dependencies. To make build process easier, you need `iroha-dev` image, which is ubuntu:16.04 with all dependencies on board.

```bash
docker build -t hyperledger/iroha-dev ${IROHA_HOME}/docker/dev 
```

Ok, you have `iroha-dev` image. It is time to use it, to build your source code:

```bash
# run dev container to build iroha
docker run -i --rm \
    # next two lines mount 2 folders from host machine to container
    -v ${IROHA_HOME}/docker/build:/build \
    -v ${IROHA_HOME}:/opt/iroha \
    hyperledger/iroha-dev \
    sh << COMMANDS
    # everything between COMMANDS will be executed inside a container
    cd /opt/iroha
    /build-iroha.sh || (echo "[-] Can't build iroha" && exit 1)
    /mktar-iroha.sh || (echo "[-] Can't make tarball" && exit 1)
    # at this step we have /tmp/iroha.tar 
    (cp /tmp/iroha.tar /build/iroha.tar || \
        (echo "[-] FAILED! Mount /build folder from your host or use iroha/docker/build.sh script!" && exit 1))
COMMANDS
```

As a result, you will have a tarball with compiled iroha binaries and libs: `${IROHA_HOME0/docker/build/iroha.tar}`.

#### 3
Then you have to build production image. It has minimal size and ready for production. The only thing you need is to upload your `sumeragi.json` inside.

```bash
# build hyperledger/iroha-docker container
docker build -t hyperledger/iroha-docker ${IROHA_HOME}/docker/build
```

#### Or, if you are lazy

Run build script and wait for completion. 
```
${IROHA_HOME}/docker/build.sh
``` 

#### Note!
In order to use iroha, you need to create `config/sumeragi.json` config file [(more)](./config-discovery/README.md).

iroha container has several scripts:
 - `/configure.sh`, which makes attempt to get config file from `configdiscovery` service.
 - `/run.sh`, which runs iroha.
 - `/configure-then-run.sh`, which runs `configure`, then `run`. 

 You can use them instead default `CMD`:
 
```bash
docker run -d --name iroha hyperledger/iroha-docker /configure-then-run.sh
```

#### Good luck!
