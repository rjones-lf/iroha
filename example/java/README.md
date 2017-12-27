# Java client library example

## Prerequisites

In order to execute the script to demonstrate java client library, you need to have java 1.8 and `gradle` build automation system installed.
Once you have the prerequisites please follow the steps:

1. Run `prepare.sh` script to build iroha java library and compile proto files:
```bash
./prepare.sh
```
2. Build java client library using gradle.

3. Make sure you have running iroha on your machine. You can follow [this guide](https://hyperledger.github.io/iroha-api/#run-the-daemon-irohad) to launch iroha daemon. Please run iroha from `iroha/example` folder, since java code uses keys from there.

## Launch example

Code in `TransactionExample.java` does the following:
1. Assembles transaction from several commands using tx builder.
2. Signs it using keys from `iroha/example` folder.
3. Sends it to iroha.
4. Waits 5 secs and checks transaction's status using it's hash.
5. Assembles query using query builder.
6. Sends query to iroha.
7. Reads query response.

Launch it using gradle:
```bash
gradle run
```
