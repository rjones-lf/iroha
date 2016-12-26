# いろは(iroha)
[![CircleCI](https://circleci.com/gh/hyperledger/iroha/tree/master.svg?style=svg)](https://circleci.com/gh/hyperledger/iroha/tree/master)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/fa0f4ce83e584fc4a32b646536dd40eb)](https://www.codacy.com?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=soramitsu/iroha&amp;utm_campaign=Badge_Grade)
[![Documentation Status](https://readthedocs.org/projects/iroha/badge/?version=latest)](http://docs.iroha.tech)

いろは (Iroha) is a simple, distributed ledger.

![alt tag](Iroha_3_sm.png)

# Pull Requests
Please include a developer certificate with pull requests: https://www.clahub.com/agreements/hyperledger/iroha

# How to build
[how_to_build](https://github.com/hyperledger/iroha/blob/master/docs/how_to_build.rst)


# Architecture (Draft)

### Directory tree
```
.
├── build (for cmake)
├── config
├── core
│   ├── consensus
│   │   └── connection
│   ├── crypto
│   ├── infra
│   │   ├── connection
│   │   ├── crypto
│   │   ├── protobuf
│   │   ├── repository
│   │   ├── server
│   │   ├── service
│   │   └── smart_contract
│   │       └── jvm
│   ├── model
│   │   ├── commands
│   │   ├── objects
│   │   ├── smart_contract
│   │   ├── state
│   │   ├── publisher
│   │   ├── repository
│   │   └── state
│   ├── publisher
│   ├── repository
│   │   ├── consensus
│   │   └── domain
│   ├── server
│   ├── service
│   ├── util
│   ├── validation
│   └── vendor
│       ├── Cappucino
│       ├── ed25519
│       ├── json
│       ├── KeccakCodePackage
│       ├── leveldb
│       └── thread_pool_cpp
├── docker
│   ├── build
│   │   └── scripts
│   ├── config-discovery
│   └── dev
│       └── scripts
├── docs
├── peer
├── smart_contract
│   └── SampleCurrency
├── test
│   ├── connection
│   ├── consensus
│   ├── crypto
│   ├── infra
│   │   ├── protobuf
│   │   └── repository
│   ├── smart_contract
│   └── vendor
├── tools
```

#### config/

It contains config file. currently we use yaml, but JSON is good like as yaml.

#### peer/

It contains main.cpp

#### smart_contract/

It contains smart_contract logic what user defines.  
Currently use java virtual machine.

#### test/

There's few tests for iroha. :bow:
We accept pull requests.

#### core/

It contains main.

```
├── connection
├── consensus
├── crypto
├── domain
│   └── transactions
├── infra
│   ├── connection
│   ├── crypto
│   ├── repository
│   ├── server
│   └── smart_contract
│       └── jvm
├── repository
├── server
├── smart_contract
├── util
└── validation
```

We adopt a **Domain-Driven Development structure** as much as possible.

```
front API
   |(1)
┌-------------┐
+ Cappuccino  +------------------------
└--+----------┘
   |                
   |                                   
┌--┼----------┐         ┌---------------┐
|  V sumeragi |(3)      |  Command exec |
|             ----------+>              |
└--|-------A--┘         └----+----------┘
   |(2)    | valdation       |
   |       └-----------------┼------┐
   |                         |      |
   |consensus building       |(4)   |
┌------------┐             ┌-V------+------┐     ┌-------┐
| connection |             |   repository  |------ model |
└------------┘             └---------------┘     └-------┘
      |                             |
Infra=|=============================|===========
      |implement                    |implement
   ┌------┐                       ┌-----┐
   | grpc |                       | D B |
   └------┘                       └-----┘

```

#### core/connection (consensus layer)
It contains the P2P messaging function interface.
```C
  void initialize_peer( std::unordered_map<std::string, std::string> config);

  bool sendAll(std::string message);
  bool send(std::string to,std::string message);
  bool receive(std::function<void(std::string from,std::string message)> callback);
```

#### core/consensus (consensus layer)
It contains the consensus algorithm(s).
  
#### core/crypto (service)
It contains digital signature algorithms, base64, hash function interfaces, etc.

#### core/model (domain)
It contains asset model, transaction logic. independent of infra knowledge.

#### core/infra (infra layer)
It contains some source depend on vendor (third party) libraries.
If any source depends on vendor libraries, it should be in infra. 

##### filenames
Filenames follow the convention: `"function"_with_"lib name".cpp`
```
connection
 └── connection_with_aeron.cpp
repository
 └── world_state_repository_with_level_db.cpp
```

#### core/server (UI layer)
It contains the server interface, currently.
```
void server();
```

#### core/smart_contract (service)
It contains the Java virtual machine interface.
```
void initializeVM(std::string contractName);
void finishVM();    
void invokeFunction(
    std::string functionName,
      std::unordered_map<std::string, std::string> params);
```

#### core/util (other)
It contains logger, random, datetime, exception...

### Environment
```
JAVA_HOME  := java's home
IROHA_HOME := iroha's root
```
  
## Requirement
```
cmake(3.5.2)
gRPC
LevelDB
```
  
## Recommended
```
fabric3 (python library, not hyperledger/fabric)
```

## Using docker and docker-compose for development
To build latest container with iroha, refer to [this guide](./docker/README.md).

After this, run network of 4 nodes:
```
docker-compose up
# open another terminal
docker-compose scale iroha=4
```

To set different number of nodes, change `command: 4` in `docker-compose.yml` and `iroha=4` in previous command.

## Installation
```
$ git submodule init
$ git submodule update
$ mkdir build
$ cd build
$ cmake ..
$ make
```
(in server)
or  
```
$ fab deploy
```
(in local)  

### Rebuilding gRPC stubs

This step should only be necessary if protobuf definitions changes, or version of gRPC or protoc has been updated.

(invoked from $IROHA_HOME)
```
protoc  --cpp_out=core/infra/connection core/infra/connection/connection.proto
protoc  --grpc_out=core/infra/connection  --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` core/infra/connection/connection.proto
```
  
## Authors 

[MakotoTakemiya](https://github.com/takemiyamakoto)  
[MizukiSonoko](https://github.com/MizukiSonoko)

## License

Copyright 2016 Soramitsu Co., Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
