Python Library
--------------

Where to Get
^^^^^^^^^^^^

There are two ways to get Iroha library for Python: via pip and manual compilation from source code. The installation via pip do the same steps as manual compilation so for both of them you need to install all of the prerequisites.

Prerequisites
"""""""""""""

CMake, git, g++, boost, swig, protobuf, python
    Please refer to the page `Installing Dependencies <dependencies.html>`__ to get installation recipes for the tools.

Install Iroha Python Libraries
""""""""""""""""""""""""""""""

- Via PIP

  .. code:: sh

      pip install iroha

  For the latest version

  .. code:: sh

      pip install -i https://testpypi.python.org/pypi iroha

- Source code

  .. code:: sh

      git clone https://github.com/hyperledger/iroha.git
      cd iroha

  For the latest version checkout to develop branch by adding *-b develop* parameter.

  .. code:: sh

      cmake -H. -Bbuild -DSWIG_PYTHON=ON -DSHARED_MODEL_DISABLE_COMPATIBILITY=ON -DSUPPORT_PYTHON2=ON;
      cmake --build build --target irohapy

      - SWIG_PYTHON=ON forces to build bindings for Python.
      - SHARED_MODEL_DISABLE_COMPATIBILITY=ON disables backward compatibility with old model of Iroha. Since you want to build only client library you don't need to have the compatibility.
      - SUPPORT_PYTHON2=ON shows that bindings will be built for Python 2. For Python 3 skip this parameter.

  After this you can find Iroha python library in **iroha/build/shared_model/bindings** folder, where you have previously cloned repository.

How to Use/Import
^^^^^^^^^^^^^^^^^

Compile Protobuf Modules of Iroha from Schema Files
"""""""""""""""""""""""""""""""""""""""""""""""""""

Iroha communicates with users through protobuf messages. In order to send transactions and queries to Iroha node you need to get python module for generating protobuf messages.
First of all you need to clone schema folder of Iroha repository. If you have already cloned Iroha repository in the previous step, just use schema folder from there.

Prerequisites
"""""""""""""

protobuf, pip
    Please refer to the page `Installing Dependencies <dependencies.html>`__ to get installation recipes for the tools.

Building Protobuf Files
"""""""""""""""""""""""

.. code:: sh

    pip install grpcio_tools
    mkdir iroha-schema
    git -C iroha-schema init
    git -C iroha-schema remote add -f schema https://github.com/hyperledger/iroha.git
    git -C iroha-schema config core.sparseCheckout true
    echo "schema" >> iroha-schema/.git/info/sparse-checkout
    git -C iroha-schema pull schema develop
    cd iroha-schema
    protoc --proto_path=schema --python_out=. block.proto primitive.proto commands.proto queries.proto responses.proto endpoint.proto
    python -m grpc_tools.protoc --proto_path=schema --python_out=. --grpc_python_out=. endpoint.proto yac.proto ordering.proto loader.proto

Protobuf files can be found in **iroha-schema** folder ('\*_pb2\*.py' files)

In order to specify Iroha libraries location:

.. code:: sh

  import sys
  sys.path.insert(0, 'path/to/iroha/libs')


Import Iroha and all of the protobuf modules that you need:

.. code:: sh

  import iroha
  import block_pb2
  import endpoint_pb2
  import endpoint_pb2_grpc
  import queries_pb2

Example Code
^^^^^^^^^^^^

.. Note::

    Work with byte arrays is different in Python 2 and Python 3. Due to this fact, the work with hashes and blobs is different in the examples. Given examples work fine with both versions of Python.

Import Iroha and schema classes, generated from Iroha protobuf:

.. code:: python

 import iroha

 import block_pb2
 import endpoint_pb2
 import endpoint_pb2_grpc
 import queries_pb2
 import grpc

Get Iroha objects:

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

Print transaction status with synchronous simple call:

.. code:: python

 def print_status(tx):
    # Create status request

    print("Hash of the transaction: ", tx.hash().hex())
    tx_hash = tx.hash().blob()

    # The work with byte arrays is different in Python 2 and 3
    # Check python version
    if sys.version_info[0] == 2:
        # Python 2 version
        tx_hash = ''.join(map(chr, tx_hash))
    else:
        # Python 3 version
        tx_hash = bytes(tx_hash)

    # Create request
    request = endpoint_pb2.TxStatusRequest()
    request.tx_hash = tx_hash

    # Create connection to Iroha
    channel = grpc.insecure_channel(IP+':'+port)
    stub = endpoint_pb2_grpc.CommandServiceStub(channel)

    # Send request
    response = stub.Status(request)
    status = endpoint_pb2.TxStatus.Name(response.tx_status)
    print("Status of transaction is:", status)

    if status != "COMMITTED":
        print("Your transaction wasn't committed")
        exit(1)

Or streaming call:

.. code:: python

    ...
    # Send request
    response = stub.StatusStream(request)

    for status in response:
        print("Status of transaction:")
        print(status)

Send transactions to Iroha:

.. code:: python

  def send_tx(tx, key_pair):
    tx_blob = proto_tx_helper.signAndAddSignature(tx, key_pair).blob()
    proto_tx = block_pb2.Transaction()

    if sys.version_info[0] == 2:
        tmp = ''.join(map(chr, tx_blob))
    else:
        tmp = bytes(tx_blob)

    proto_tx.ParseFromString(tmp)

    channel = grpc.insecure_channel(IP+':'+port)
    stub = endpoint_pb2_grpc.CommandServiceStub(channel)

    stub.Torii(proto_tx)

Send query to Iroha and receive a responce:

.. code:: python

  def send_query(query, key_pair):
    query_blob = proto_query_helper.signAndAddSignature(query, key_pair).blob()

    proto_query = queries_pb2.Query()

    if sys.version_info[0] == 2:
        tmp = ''.join(map(chr, query_blob))
    else:
        tmp = bytes(query_blob)

    proto_query.ParseFromString(tmp)

    channel = grpc.insecure_channel(IP+':'+port)
    query_stub = endpoint_pb2_grpc.QueryServiceStub(channel)
    query_response = query_stub.Find(proto_query)

    return query_response

Create domain and asset:

.. code:: python

  tx = tx_builder.creatorAccountId(creator) \
        .createdTime(current_time) \
        .createDomain("domain", "user") \
        .createAsset("coin", "domain", 2).build()

  send_tx(tx, key_pair)
  print_status(tx)

Create asset quantity:

.. code:: python

  tx = tx_builder.creatorAccountId(creator) \
        .createdTime(current_time) \
        .addAssetQuantity("admin@test", "coin#domain", "1000.2").build()

  send_tx(tx, key_pair)
  print_status(tx)

Create account:

.. code:: python

  user1_kp = crypto.generateKeypair()

  tx = tx_builder.creatorAccountId(creator) \
        .createdTime(current_time) \
        .createAccount("userone", "domain", user1_kp.publicKey()).build()

  send_tx(tx, key_pair)
  print_status(tx)

Send asset:

.. code:: python

  tx = tx_builder.creatorAccountId(creator) \
        .createdTime(current_time) \
        .transferAsset("admin@test", "userone@domain", "coin#domain", "Some message", "2.0").build()

  send_tx(tx, key_pair)
  print_status(tx)

Get asset info:

.. code:: python

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

Get account asset:

.. code:: python

    query = query_builder.creatorAccountId(creator) \
        .createdTime(current_time) \
        .queryCounter(11) \
        .getAccountAssets("userone@domain", "coin#domain") \
        .build()

    query_response = send_query(query, key_pair)

    print(query_response)
