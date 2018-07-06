import sys
sys.path.insert(0, 'build/shared_model/bindings')
import iroha

import endpoint_pb2_grpc
import queries_pb2
import grpc
import time


blocks_query_builder = iroha.ModelBlocksQueryBuilder()
crypto = iroha.ModelCrypto()

admin_priv = open("../admin@test.priv", "r").read()
admin_pub = open("../admin@test.pub", "r").read()
key_pair = crypto.convertFromExisting(admin_pub, admin_priv)

creator = "admin@test"
current_time = int(round(time.time() * 1000)) - 10**5

def get_blocks():
    query = blocks_query_builder.creatorAccountId(creator)\
    .createdTime(current_time)\
    .queryCounter(1) \
    .build()

    query_blob = iroha.ModelProtoBlocksQuery(query).signAndAddSignature(key_pair).finish().blob()
    proto_query = queries_pb2.BlocksQuery()

    if sys.version_info[0] == 2:
        tmp = ''.join(map(chr, query_blob))
    else:
        tmp = bytes(query_blob)

    proto_query.ParseFromString(tmp)

    channel = grpc.insecure_channel('127.0.0.1:50051')
    query_stub = endpoint_pb2_grpc.QueryServiceStub(channel)
    query_response = query_stub.FetchCommits(proto_query)

    for block in query_response:
        print("block:")
        print(block)

get_blocks()
