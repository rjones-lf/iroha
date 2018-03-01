# NodeJS client library example

## Prerequisites

In order to execute script demonstrating execution of NodeJS client library you need to have NodeJS installed. Any version is appropriate, however, to build and use client library for NodeJS version higher than 7, you have to install SWIG 3.0.12 with [this patch](https://github.com/swig/swig/pull/968.patch).

1. Run prepare.sh script to build iroha python library and compile proto files:
```sh
./prepare.sh
```
  For Mac users, please use (this way the variable `DYLD_LIBRARY_PATH` will be exported into current terminal session, allowing running script)
```sh
. ./prepare.sh
```

2. Make sure you have running iroha on your machine. You can follow [this](https://hyperledger.github.io/iroha-api/#run-the-daemon-irohad) guide to launch iroha daemon. Please run iroha from iroha/example folder, since python script uses keys from there.

## Launch example

Script `index.js` does the following:
1. Assemble transaction from several commands using tx builder
2. Sign it using keys from iroha/example folder
3. Send it to iroha
4. Wait 5 secs and check transaction's status using its hash
5. Assemble query using query builder
6. Send query to iroha
7. Read query response

Launch it:
```sh
node index.js
```
