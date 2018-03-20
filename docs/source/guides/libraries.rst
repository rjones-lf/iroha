Client libraries
================

C++ library
-----------

Where to get
^^^^^^^^^^^^

How to use/import
^^^^^^^^^^^^^^^^^

Example code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^

Java library
------------

Where to get
^^^^^^^^^^^^

How to use/import
^^^^^^^^^^^^^^^^^

Example code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^

Objective-C library
-------------------

Where to get
^^^^^^^^^^^^

How to use/import
^^^^^^^^^^^^^^^^^

Example code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^

Swift library
-------------

Where to get
^^^^^^^^^^^^

How to use/import
^^^^^^^^^^^^^^^^^

Example code
^^^^^^^^^^^^

Troubleshoting
^^^^^^^^^^^^^^

Python library
--------------

Where to get
^^^^^^^^^^^^

Prerequirements:

- cmake(3.11 or higher)
- git
- g++
- boost(1.65 or higher, only system and filesystem)
- swig(3.0.12 can be built --without-pcre)
- protobuf

Install iroha python libraries:

- Through pip

    .. code:: sh

      pip install iroha

    For the latest version

    .. code:: sh

      pip install -i https://testpypi.python.org/pypi iroha

- Source code

    .. code:: sh

      git clone https://github.com/hyperledger/iroha.git
      cd iroha

    For the latest version checkout to develop branch

    .. code:: sh

      git checkout develop

    .. code:: sh

      cmake -H. -Bbuild -DSWIG_PYTHON=ON -DSHARED_MODEL_DISABLE_COMPATIBILITY=ON -DSUPPORT_PYTHON2=ON;
      cmake --build build -- -j4

    After this you can find iroha python library in **iroha/build/shared_model/bindings** folder.

Install iroha protobuf files:

  First of all you need to clone iroha repository

  .. code:: sh

      pip install grpcio_tools
      git clone https://github.com/hyperledger/iroha.git
      cd iroha
      protoc --proto_path=schema --python_out=build block.proto primitive.proto commands.proto queries.proto responses.proto endpoint.proto
      python -m grpc_tools.protoc --proto_path=schema --python_out=build --grpc_python_out=build endpoint.proto yac.proto ordering.proto loader.proto

  Protobuf files can be found in **iroha/build** folder ('\*_pb2\*.py' files)

How to use/import
^^^^^^^^^^^^^^^^^

In order to specify iroha libraries location:

.. code:: sh

  import sys
  sys.path.insert(0, 'path/to/iroha/libs')


Import iroha and all of the protobuf modules that you need:

.. code:: sh

  import iroha
  import block_pb2
  import endpoint_pb2
  import endpoint_pb2_grpc
  import queries_pb2

Example code
^^^^^^^^^^^^

Note: work with raw data can be different in Python 2 and Python 3.

Import iroha and irohas protobuf:

.. code:: python

 import iroha

 import block_pb2
 import endpoint_pb2
 import endpoint_pb2_grpc
 import queries_pb2
 import grpc

Get iroha objects:

.. code:: python

 tx_builder = iroha.ModelTransactionBuilder()
 query_builder = iroha.ModelQueryBuilder()
 crypto = iroha.ModelCrypto()
 proto_tx_helper = iroha.ModelProtoTransaction()
 proto_query_helper = iroha.ModelProtoQuery()

Read public and private keys:

.. code:: python

 admin_priv = open("admin@test.priv", "r").read()
 admin_pub = open("admin@test.pub", "r").read()
 key_pair = crypto.convertFromExisting(admin_pub, admin_priv)

Get status of transaction:

.. code:: python

 def print_status(tx):
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
    channel = grpc.insecure_channel(IP+':50051')
    stub = endpoint_pb2_grpc.CommandServiceStub(channel)

    # Send request
    response = stub.Status(request)
    status = endpoint_pb2.TxStatus.Name(response.tx_status)
    print("Status of transaction is:", status)

    if status != "COMMITTED":
        print("Your transaction wasn't committed")
        exit(1)

Send transactions to iroha:

.. code:: python

  def send_tx(tx, key_pair):
    tx_blob = proto_tx_helper.signAndAddSignature(tx, key_pair).blob()
    proto_tx = block_pb2.Transaction()

    if sys.version_info[0] == 2:
        tmp = ''.join(map(chr, tx_blob))
    else:
        tmp = bytes(tx_blob)

    proto_tx.ParseFromString(tmp)

    channel = grpc.insecure_channel(IP+':50051')
    stub = endpoint_pb2_grpc.CommandServiceStub(channel)

    stub.Torii(proto_tx)

Create domain and asset:

.. code:: python

  tx = tx_builder.creatorAccountId(creator) \
        .txCounter(tx_counter) \
        .createdTime(current_time) \
        .createDomain("domain", "user") \
        .createAsset("coin", "domain", 2).build()

  send_tx(tx, key_pair)
  print_status(tx)

Create asset quantity:

.. code:: python

  tx = tx_builder.creatorAccountId(creator) \
        .txCounter(tx_counter) \
        .createdTime(current_time) \
        .addAssetQuantity("admin@test", "coin#domain", "1000.2").build()

  send_tx(tx, key_pair)
  print_status(tx)

Create account:

.. code:: python

  user1_kp = crypto.generateKeypair()

  tx = tx_builder.creatorAccountId(creator) \
        .txCounter(tx_counter) \
        .createdTime(current_time) \
        .createAccount("user1", "domain", user1_kp.publicKey()).build()

  send_tx(tx, key_pair)
  print_status(tx)

Send asset:

.. code:: python

  tx = tx_builder.creatorAccountId(creator) \
        .txCounter(tx_counter) \
        .createdTime(current_time) \
        .transferAsset("admin@test", "user1@domain", "coin#domain", "Some message", "2.0").build()

  send_tx(tx, key_pair)
  print_status(tx)

Get asset amount:

.. code:: python

    query = query_builder.creatorAccountId(creator) \
        .createdTime(current_time) \
        .queryCounter(start_query_counter) \
        .getAssetInfo("user1#domain") \
        .build()
    query_blob = proto_query_helper.signAndAddSignature(query, key_pair).blob()

    proto_query = queries_pb2.Query()

    if sys.version_info[0] == 2:
        tmp = ''.join(map(chr, query_blob))
    else:
        tmp = bytes(query_blob)

    proto_query.ParseFromString(tmp)

    query_stub = endpoint_pb2_grpc.QueryServiceStub(channel)
    query_response = query_stub.Find(proto_query)

    if not query_response.HasField("asset_response"):
        print("Query response error")
        exit(1)
    else:
        print("Query responded with asset response")

    asset_info = query_response.asset_response.asset
    print("Asset Id =", asset_info.asset_id)
    print("Precision =", asset_info.precision)

Troubleshooting
^^^^^^^^^^^^^^^

NodeJS library
--------------

Where to get
^^^^^^^^^^^^

How to use/import
^^^^^^^^^^^^^^^^^

Example code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^
