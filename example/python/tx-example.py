import sys
sys.path.insert(0, 'build/shared_model/bindings')
import iroha

import transaction_pb2
import endpoint_pb2
import endpoint_pb2_grpc
import queries_pb2
import grpc
import time


tx_builder = iroha.ModelTransactionBuilder()
query_builder = iroha.ModelQueryBuilder()
crypto = iroha.ModelCrypto()

admin_priv = open("../admin@test.priv", "r").read()
admin_pub = open("../admin@test.pub", "r").read()
key_pair = crypto.convertFromExisting(admin_pub, admin_priv)

user1_kp = crypto.generateKeypair()

def current_time():
    return int(round(time.time() * 1000))

creator = "admin@test"

query_counter = 1

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
    tx_blob = iroha.ModelProtoTransaction(tx).signAndAddSignature(key_pair).finish().blob()
    proto_tx = transaction_pb2.Transaction()

    if sys.version_info[0] == 2:
        tmp = ''.join(map(chr, tx_blob))
    else:
        tmp = bytes(tx_blob)

    proto_tx.ParseFromString(tmp)

    channel = grpc.insecure_channel('127.0.0.1:50051')
    stub = endpoint_pb2_grpc.CommandServiceStub(channel)

    stub.Torii(proto_tx)


def send_query(query, key_pair):
    query_blob = iroha.ModelProtoQuery(query).signAndAddSignature(key_pair).finish().blob()

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


def create_asset_coin():
    """
    Create domain "domain" and asset "coin#domain" with precision 2
    """
    tx = tx_builder.creatorAccountId(creator) \
            .createdTime(current_time()) \
            .createDomain("domain", "user") \
            .createAsset("coin", "domain", 2).build()

    send_tx(tx, key_pair)
    print_status_streaming(tx)


def add_coin_to_admin():
    """
    Add 1000.00 asset quantity of asset coin to admin
    """
    tx = tx_builder.creatorAccountId(creator) \
        .createdTime(current_time()) \
        .addAssetQuantity("coin#domain", "1000.00").build()

    send_tx(tx, key_pair)
    print_status_streaming(tx)


def create_account_userone():
    """
    Create account "userone@domain"
    """
    tx = tx_builder.creatorAccountId(creator) \
        .createdTime(current_time()) \
        .createAccount("userone", "domain", user1_kp.publicKey()).build()

    send_tx(tx, key_pair)
    print_status_streaming(tx)

def transfer_coin_from_admin_to_userone():
    """
    Transfer 2.00 of coin from admin@test to userone@domain
    """
    tx = tx_builder.creatorAccountId(creator) \
        .createdTime(current_time()) \
        .transferAsset("admin@test", "userone@domain", "coin#domain", "Some message", "2.00").build()

    send_tx(tx, key_pair)
    print_status_streaming(tx)

def grant_admin_to_add_detail_to_userone():
    """
    Grant admin@test to be able to set details information to userone@domain
    """
    tx = tx_builder.creatorAccountId("userone@domain") \
        .createdTime(current_time()) \
        .grantPermission(creator, iroha.Grantable_kSetMyAccountDetail) \
        .build()

    send_tx(tx, user1_kp)
    print_status_streaming(tx)

def set_age_to_userone_by_admin():
    """
    Set age to userone@domain by admin@test
    """
    tx = tx_builder.creatorAccountId(creator) \
        .createdTime(current_time()) \
        .setAccountDetail("userone@domain", "age", "18") \
        .build()

    send_tx(tx, key_pair)
    print_status_streaming(tx)

def get_coin_info():
    """
    Get information about asset coin#domain
    """
    global query_counter
    query_counter += 1
    query = query_builder.creatorAccountId(creator) \
        .createdTime(current_time()) \
        .queryCounter(query_counter) \
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
    """
    Get list of transactions done by userone@domain with asset coin#domain
    """
    global query_counter
    query_counter += 1
    query = query_builder.creatorAccountId(creator) \
        .createdTime(current_time()) \
        .queryCounter(query_counter) \
        .getAccountAssets("userone@domain") \
        .build()

    query_response = send_query(query, key_pair)

    print(query_response)

def get_userone_info():
    """
    Get userone's key value information
    """
    global query_counter
    query_counter += 1
    query = query_builder.creatorAccountId(creator) \
        .createdTime(current_time()) \
        .queryCounter(query_counter) \
        .getAccountDetail("userone@domain") \
        .build()

    query_response = send_query(query, key_pair)
    print(query_response.account_detail_response.detail)



create_asset_coin()
add_coin_to_admin()
create_account_userone()
transfer_coin_from_admin_to_userone()
grant_admin_to_add_detail_to_userone()
set_age_to_userone_by_admin()
get_coin_info()
get_account_asset()
get_userone_info()
print("done!")
