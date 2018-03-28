Client Libraries
================

C++ Library
-----------

Where to Get
^^^^^^^^^^^^

How to Use/Import
^^^^^^^^^^^^^^^^^

Example Code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^

Java Library
------------

Where to Get
^^^^^^^^^^^^

How to Use/Import
^^^^^^^^^^^^^^^^^

Example Code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^

Android Library
---------------

The library, in essence, is a set of Java interfaces and binary libraries compiled for different architectures. Supported architectures are *arm, x86, x86_64*.

Where to Get
^^^^^^^^^^^^

There are two ways to get Iroha library for Android:

#. Grab via Gradle (see details in the section `Importing the Library from jcenter`_)

   .. code-block:: groovy

       implementation 'jp.co.soramitsu.iroha.android:iroha-android-bindings:1.0'

#. Compile the library on your own.

Both options are described in the following sections.

Manual Build
^^^^^^^^^^^^

The guide was tested on systems running Ubuntu 16.04 and macOS.

Prerequisites
"""""""""""""

Android NDK
    Please `download <https://developer.android.com/ndk/downloads/index.html>`__ and unpack NDK to any suitable folder.

Automake, Bison, Cmake
    Please refer to the page `Installing Dependencies <dependencies.html>`__ to get installation recipes for the tools.

Building the Library
""""""""""""""""""""

All you need now is to download `build script <https://github.com/hyperledger/iroha/blob/develop/example/Android/android-build.sh>`__
``android-build.sh`` to any empty folder and launch it there.

Launch parameters are listed in the table below.

+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| Position | Required | Parameter Name        | Description                                 | Possible Values                     |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| 1        | Yes      | **Platform Name**     | Name of the target platform for binary      | ``arm64-v8a``, ``armeabi-v7a``,     |
|          |          |                       | part of the library.                        | ``armeabi``, ``x86``, ``x86_64``    |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| 2        | Yes      | **Android API Level** | API level supported by your NDK.            | ``27`` for android-ndk-r16b         |
|          |          |                       | See the link under the table for details.   |                                     |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| 3        | Yes      | **Android NDK Path**  | Full path to unpacked NDK. Please           | ``/home/user/lib/android-ndk-r16b`` |
|          |          |                       | ensure that path does not contain spaces.   |                                     |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| 4        | Yes      | **Java Package Name** | Package name that will be used for Java     | ``jp.co.soramitsu.iroha.android``   |
|          |          |                       | interfaces generation. Note that the binary |                                     |
|          |          |                       | also depends on chosen package name.        |                                     |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| 5        | No       | **Build Type**        | Defines build mode of binary part           | ``Debug`` or ``Release``            |
|          |          |                       | of the library. ``Release`` is the default  |                                     |
|          |          |                       | option.                                     |                                     |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+

`Android API levels <https://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels>`__

Please use the same root part of Java package name for library build as you use for your Android project.
For example, your project is located in a package called ``com.mycompany.androidapp``, so please consider to build the library in a
package, which name starts with ``com.mycompany.androidapp`` (e.g. ``com.mycompany.androidapp.iroha``).

A couple of launch commands examples:

.. code-block:: shell

    # build Java bindings and binary library for arm64-v8a in Release mode
    ./android-build.sh arm64-v8a 27 /home/user/lib/android-ndk-r16b com.mycompany.iroha

    # build Java bindings and binary library for x86 in Debug mode
    ./android-build.sh x86 27 /home/user/lib/android-ndk-r16b com.mycompany.iroha Debug

Build artefacts will be collected in ``lib`` directory near the script ``android-build.sh``.
There will be two files - an archive ``bindings.zip`` and ``libirohajava.so``.


How to Use/Import
^^^^^^^^^^^^^^^^^

Importing the Library from jcenter
""""""""""""""""""""""""""""""""""

The easiest way to use Irohalib for Android is to import the library dependency from `jcenter <https://bintray.com/bulatmukhutdinov/maven/iroha-android-bindings>`__.

All you need to do is a simple set of four steps:

1. Add to your ``build.gradle`` file the following line:

   .. code-block:: groovy

       implementation 'jp.co.soramitsu.iroha.android:iroha-android-bindings:1.0'

2. Copy the latest version of ``*.proto`` files from ``develop`` branch of Iroha `repository <https://github.com/hyperledger/iroha/tree/develop/schema>`__ into
   ``app/src/main/proto/`` folder inside your project in Android Studio.

   The resulting directory structure should look like as follows:

   .. code-block:: shell

        app
        └── src
            └── main
                └── proto
                    ├── google
                    │   └── protobuf
                    │       └── empty.proto
                    ├── block.proto
                    ├── commands.proto
                    ├── endpoint.proto
                    ├── loader.proto
                    ├── ordering.proto
                    ├── primitive.proto
                    ├── proposal.proto
                    ├── queries.proto
                    ├── responses.proto
                    └── yac.proto


3. Create additional directories ``app/src/main/proto/google/protobuf/`` and place there a file called ``empty.proto`` with the following contents:

   .. code-block:: proto

       syntax = "proto3";

       package google.protobuf;

       option java_package = "com.google.protobuf";
       option java_outer_classname = "EmptyProto";
       option java_multiple_files = true;

       message Empty {
       }

4. Add ``protobuf`` and ``grpc`` dependecies and protobuf configuration block into your ``buld.gradle`` file.

   .. code-block:: groovy

        apply plugin: 'com.google.protobuf'

        dependencies {
            ...

            implementation 'com.google.protobuf:protobuf-lite:3.0.1'
            implementation 'io.grpc:grpc-core:1.8.0'
            implementation 'io.grpc:grpc-stub:1.8.0'
            implementation 'io.grpc:grpc-okhttp:1.8.0'
            implementation('io.grpc:grpc-protobuf-lite:1.8.0') {
            // Otherwise Android compile will complain "Multiple dex files define ..."
            exclude module: "protobuf-lite"
        }

        protobuf {
            protoc {
                artifact = 'com.google.protobuf:protoc:3.5.1-1'
            }
            plugins {
                javalite {
                    artifact = "com.google.protobuf:protoc-gen-javalite:3.0.0"
                }
                grpc {
                    artifact = 'io.grpc:protoc-gen-grpc-java:1.10.0'
                }
            }
            generateProtoTasks {
                all().each { task ->
                    task.plugins {
                        javalite {}
                        grpc {
                            // Options added to --grpc_out
                            option 'lite'
                            option 'generate_equals=true'
                        }
                    }
                }
            }
        }

How to Use Manually Built Library
"""""""""""""""""""""""""""""""""

1. Create directory structure inside your Android project according to the package name of build library.
   Put there all the ``.java`` files from ``bindings.zip`` archive.
   For example, the path could be ``app/src/main/java/com/mycompany/iroha`` if you built the library with
   ``com.mycompany.iroha`` package name.

2. Create directory ``app/src/main/jniLibs/<platform>`` where ``<platform>`` is the name of target platform
   (e.g. ``arm64-v8a``). Put there ``libirohajava.so``. Repeat this step for all required platforms
   (in this case you need to build the library for each platform).

3. Repeat steps 2-4 from the previous section `Importing the Library from jcenter`_.


Example Code
^^^^^^^^^^^^

Explore ``bindings`` branch of `iroha-android <https://github.com/hyperledger/iroha-android/tree/bindings>`__ repository to get source code and view sample application.


Objective-C Library
-------------------

Where to Get
^^^^^^^^^^^^

How to Use/Import
^^^^^^^^^^^^^^^^^

Example Code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^

Swift Library
-------------

Where to Get
^^^^^^^^^^^^

How to Use/Import
^^^^^^^^^^^^^^^^^

Example Code
^^^^^^^^^^^^

Troubleshoting
^^^^^^^^^^^^^^

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
        .createAccount("userone", "domain", user1_kp.publicKey()).build()

  send_tx(tx, key_pair)
  print_status(tx)

Send asset:

.. code:: python

  tx = tx_builder.creatorAccountId(creator) \
        .txCounter(tx_counter) \
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

NodeJS Library
--------------

Where to Get
^^^^^^^^^^^^

How to Use/Import
^^^^^^^^^^^^^^^^^

Example Code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^
