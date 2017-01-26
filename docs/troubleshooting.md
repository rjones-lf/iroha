# Troubleshooting

## Rebuilding gRPC stubs

This step should only be necessary if protobuf definitions changes, or version of gRPC or protoc has been updated.

(invoked from $IROHA_HOME)
```
protoc  --cpp_out=core/infra/connection core/infra/connection/connection.proto
protoc  --grpc_out=core/infra/connection  --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` core/infra/connection/connection.proto
```

## Issues

If you noticed any bug, please [open an issue](https://github.com/hyperledger/iroha/issues).