import sys
sys.path.insert(0, 'build/shared_model/bindings')
import iroha

import block_pb2
import endpoint_pb2
import endpoint_pb2_grpc
import queries_pb2
import grpc
import time


tx_builder = iroha.ModelTransactionBuilder()
query_builder = iroha.ModelQueryBuilder()
crypto = iroha.ModelCrypto()
proto_tx_helper = iroha.ModelProtoTransaction()
proto_query_helper = iroha.ModelProtoQuery()

admin_priv = open("../admin@test.priv", "r").read()
admin_pub = open("../admin@test.pub", "r").read()
key_pair = crypto.convertFromExisting(admin_pub, admin_priv)

current_time = int(round(time.time() * 1000)) - 10**5
creator = "admin@test"

def get_status(tx):
    # Create status request

    print("Hash of the transaction: ", tx.hash().hex())
    tx_hash = tx.hash().blob()

    if sys.version_info[0] == 2:
        tx_hash = ''.join(map(chr, tx_hash))
    else:
        tx_hash = bytes(tx_hash)


    request = endpoint_pb2.TxStatusRequest()
    request.tx_hash = tx_hash

    channel = grpc.insecure_channel('127.0.0.1:50051')
    stub = endpoint_pb2_grpc.CommandServiceStub(channel)

    response = stub.Status(request)
    status = endpoint_pb2.TxStatus.Name(response.tx_status)
    print("Status of transaction is:", status)

    if status != "COMMITTED":
        print("Your transaction wasn't committed")
        exit(1)


def print_status_streaming(tx):
    # Create status request

    print("Hash of the transaction: ", tx.hash().hex())
    tx_hash = tx.hash().blob()

    # Check python version
    if sys.version_info[0] == 2:
        tx_hash = ''.join(map(chr, tx_hash))
    else:
        tx_hash = bytes(tx_hash)

    # Create request
    request = endpoint_pb2.TxStatusRequest()
    request.tx_hash = tx_hash

    # Create connection to Iroha
    channel = grpc.insecure_channel('127.0.0.1:50051')
    stub = endpoint_pb2_grpc.CommandServiceStub(channel)

    # Send request
    response = stub.StatusStream(request)

    for status in response:
        print("Status of transaction:")
        print(status)


def send_tx(tx, key_pair):
    tx_blob = proto_tx_helper.signAndAddSignature(tx, key_pair).blob()
    proto_tx = block_pb2.Transaction()

    if sys.version_info[0] == 2:
        tmp = ''.join(map(chr, tx_blob))
    else:
        tmp = bytes(tx_blob)

    proto_tx.ParseFromString(tmp)

    channel = grpc.insecure_channel('127.0.0.1:50051')
    stub = endpoint_pb2_grpc.CommandServiceStub(channel)

    stub.Torii(proto_tx)


def send_query(query, key_pair):
    query_blob = proto_query_helper.signAndAddSignature(query, key_pair).blob()

    proto_query = queries_pb2.Query()

    if sys.version_info[0] == 2:
        tmp = ''.join(map(chr, query_blob))
    else:
        tmp = bytes(query_blob)

    proto_query.ParseFromString(tmp)

    channel = grpc.insecure_channel('127.0.0.1:50051')
    query_stub = endpoint_pb2_grpc.QueryServiceStub(channel)
    query_response = query_stub.Find(proto_query)

    return query_response


def tx1():
    tx = tx_builder.creatorAccountId(creator) \
            .createdTime(current_time) \
            .createDomain("domain", "user") \
            .createAsset("coin", "domain", 2).build()

    send_tx(tx, key_pair)
    print_status_streaming(tx)


def tx2():
    tx = tx_builder.creatorAccountId(creator) \
        .createdTime(current_time) \
        .addAssetQuantity("admin@test", "coin#domain", "1000.00").build()

    send_tx(tx, key_pair)
    print_status_streaming(tx)


def tx3():
    user1_kp = crypto.generateKeypair()

    tx = tx_builder.creatorAccountId(creator) \
        .createdTime(current_time) \
        .createAccount("userone", "domain", user1_kp.publicKey()).build()

    send_tx(tx, key_pair)
    print_status_streaming(tx)


def tx4():
    tx = tx_builder.creatorAccountId(creator) \
        .createdTime(current_time) \
        .transferAsset("admin@test", "userone@domain", "coin#domain", "Some message", "2.00").build()

    send_tx(tx, key_pair)
    print_status_streaming(tx)


def get_asset():
    query = query_builder.creatorAccountId(creator) \
        .createdTime(current_time) \
        .queryCounter(1) \
        .getAssetInfo("coin#domain") \
        .build()

    query_response = send_query(query, key_pair)

    if not query_response.HasField("asset_response"):
        print("Query response error")
        exit(1)
    else:
        print("Query responded with asset response")

    asset_info = query_response.asset_response.asset
    print("Asset Id =", asset_info.asset_id)
    print("Precision =", asset_info.precision)


def get_account_asset():
    query = query_builder.creatorAccountId(creator) \
        .createdTime(current_time) \
        .queryCounter(11) \
        .getAccountAssets("userone@domain", "coin#domain") \
        .build()

    query_response = send_query(query, key_pair)

    print(query_response)


tx1()
tx2()
tx3()
tx4()
get_asset()
get_account_asset()
print("done!")
