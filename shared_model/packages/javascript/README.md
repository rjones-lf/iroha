# iroha-js

Official Iroha JavaScript Library. https://github.com/hyperledger/iroha

## Usage

You can use regular Node.js style to import **iroha-lib** package and related protobufs:

```javascript
const iroha = require('iroha-lib')

const blockTransaction = require('iroha-lib/pb/block_pb.js').Transaction
const endpointGrpc = require('iroha-lib/pb/endpoint_grpc_pb.js')

...

```

Watch usage in *example* folder.

## Build

You need this section if you want to build **iroha-lib** manually for publishing or if your architecture/OS not supported yet.

### Prerequisities

**
WARNING!
If you have already installed SWIG, you MUST install patched version instead using [this patch](https://github.com/swig/swig/pull/968.patch).
Or just delete global installed SWIG - Iroha be able to pull and compile it automatically.
**

In order to build NPM package by `node-gyp` on your machine you need some global installed dependencies: 

1. CMake (>=3.8.2)

2. Protobuf (>=3.5.1)

3. Boost (>=1.65.1)

#### For Mac users

To build **iroha-lib** on Mac the following dependencies should be installed:

```sh
brew install node cmake # Common dependencies
brew install autoconf automake ccache # SWIG dependencies
brew install protobuf boost # Iroha dependencies
```

### Build process

1. Clone full Iroha repository

```sh
git clone -b develop --depth=1 https://github.com/hyperledger/iroha

```

2. Go to the NPM package directory and start build

```sh
cd iroha/shared_model/packages/javascript
npm install
```

That's pretty all.

---


This NPM package is in deep pre-alfa phase, so if you have any troubles, feel free to create a new issue or contact contributors from *package.json*.
