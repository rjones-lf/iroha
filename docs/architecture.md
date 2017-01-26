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


## Environment
```
JAVA_HOME  := java's home
IROHA_HOME := iroha's root
```

## Requirements
```
cmake(3.5.2)
gRPC
LevelDB
```