# How to build `iroha` using docker

Clone `iroha.git` on your directory.

```
git clone https://github.com/hyperledger/iroha.git
cd iroha
```

Build depends on the environment variable `IROHA_HOME` so you need to set it:

`export IROHA_HOME=$(pwd)`

Run build script and wait for completion. 
```
${IROHA_HOME}/docker/build.sh
``` 



