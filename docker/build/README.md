# Info

First of all, you have to get the latest iroha binaries. For this, refer to [iroha-build](../README.md). Basically, you need to run `../build.sh`. It creates `iroha.tar` in this folder.

This directory will be mounted to `iroha-dev` container. After successful build, `iroha.tar` will be copied to host computer to this folder.

Then, to build iroha container, run:
```
docker build -t hyperledger/iroha .
```
