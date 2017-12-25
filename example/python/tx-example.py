from __future__ import print_function

import sys
# import iroha library from nested folder
sys.path.insert(0, 'shared_model/bindings')
import iroha

import time
import block_pb2
import endpoint_pb2
import endpoint_pb2_grpc
import queries_pb2
import grpc

txbuilder = iroha.ModelTransactionBuilder()
queryBuilder = iroha.ModelQueryBuilder()
crypto = iroha.ModelCrypto()
protoTxHelper = iroha.ModelProtoTransaction()
protoQueryHelper = iroha.ModelProtoQuery()

admin_priv = open("../admin@test.priv", "r").read()
admin_pub = open("../admin@test.pub", "r").read()

me_kp = crypto.convertFromExisting(admin_pub, admin_priv)

current_time = int(round(time.time() * 1000)) - 10**5
startCounter = 1
creator = "admin@test"


# build transaction
tx = txbuilder.creatorAccountId(creator) \
    .createdTime(current_time) \
    .createDomain("ru", "user") \
    .createAsset("dollar", "ru", 2).build()

tx_blob = protoTxHelper.signAndAddSignature(tx, me_kp).blob()

# create proto object and send to iroha

proto_tx = block_pb2.Transaction()
proto_tx.ParseFromString(''.join(map(chr, tx_blob)))

channel = grpc.insecure_channel('127.0.0.1:50051')
stub = endpoint_pb2_grpc.CommandServiceStub(channel)

stub.Torii(proto_tx)

time.sleep(5)

# create status request
print("Hash of the transaction: ", tx.hash().hex())
tx_hash = tx.hash().blob()
tx_hash = ''.join(map(chr, tx_hash))

request = endpoint_pb2.TxStatusRequest()
request.tx_hash = tx_hash

response = stub.Status(request)
status = endpoint_pb2.TxStatus.Name(response.tx_status)
print("Status of transaction is:", status)

if status != "COMMITTED":
    print("Your transaction wasn't committed")
    exit()

query = queryBuilder.creatorAccountId(creator) \
    .createdTime(current_time) \
    .getAssetInfo("dollar#ru") \
    .build()
query_blob = protoQueryHelper.signAndAddSignature(query, me_kp).blob()

proto_query = queries_pb2.Query()
proto_query.ParseFromString(''.join(map(chr, query_blob)))

query_stub = endpoint_pb2_grpc.QueryServiceStub(channel)
query_response = query_stub.Find(proto_query)

if not query_response.HasField("asset_response"):
    print("Query response error")
    exit(0)
else:
    print("Query responsed with asset response")

asset_info = query_response.asset_response.asset
print("Asset Id =", asset_info.asset_id)
print("Precision =", asset_info.precision)

print("done!")
