import iroha

import time
import block_pb2
import endpoint_pb2
import endpoint_pb2_grpc
import grpc

txbuilder = iroha.ModelTransactionBuilder()
queryBuilder = iroha.ModelQueryBuilder()
crypto = iroha.ModelCrypto()
protoTxHelper = iroha.ModelProtoTransaction()
protoQueryHelper = iroha.ModelProtoQuery()

me_kp = crypto.convertFromExisting("407e57f50ca48969b08ba948171bb2435e035d82cec417e18e4a38f5fb113f83", "1d7e0a32ee0affeb4d22acd73c2c6fb6bd58e266c8c2ce4fa0ffe3dd6a253ffb")

print(me_kp.publicKey().hex())

peer_kp = crypto.generateKeypair()
signatory_kp = crypto.generateKeypair()
account_kp = crypto.generateKeypair()
current_time = int(round(time.time() * 1000)) - 10**5
startCounter = 1
creator = "admin@test"
# signatory = "fyodor@iroha"


# build transaction
tx = txbuilder.creatorAccountId(creator) \
    .createdTime(current_time) \
    .createDomain("ru", "user").build()

tx_blob = protoTxHelper.signAndAddSignature(tx, me_kp).blob()

# create proto object and send to iroha

proto_tx = block_pb2.Transaction()
proto_tx.ParseFromString(''.join(map(chr, tx_blob)))

channel = grpc.insecure_channel('127.0.0.1:50051')
stub = endpoint_pb2_grpc.CommandServiceStub(channel)

stub.Torii(proto_tx)

time.sleep(2)

# create status request
print("Hash in hex", tx.hash().hex())
tx_hash = tx.hash().blob()
tx_hash = ''.join(map(chr, tx_hash))
print "Tx hash is", tx_hash

request = endpoint_pb2.TxStatusRequest()
request.tx_hash = tx_hash

response = stub.Status(request)

print(response.tx_status)

print("done!")
