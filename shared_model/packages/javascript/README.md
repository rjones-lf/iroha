# iroha-js

Official Iroha JavaScript Library. https://github.com/hyperledger/iroha

This package fully compatible with [Iroha 1.0 beta-3](https://github.com/hyperledger/iroha/releases/tag/v1.0.0_beta-3)

## Usage

You can use regular Node.js style to import **iroha-lib** package and related protobufs:

```javascript
const iroha = require('iroha-lib')

const blockTransaction = require('iroha-lib/pb/block_pb.js').Transaction
const endpointGrpc = require('iroha-lib/pb/endpoint_grpc_pb.js')

...

```

Watch usage in *example* folder and read the [docs](https://iroha.readthedocs.io/en/latest/guides/libraries/nodejs.html) .


---


This NPM package is in deep pre-alfa phase, so if you have any troubles, feel free to create a new issue or contact contributors from *package.json*.
