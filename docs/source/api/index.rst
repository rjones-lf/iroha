Iroha API reference
===================

In API section we will take a look at building blocks of an application interacting with Iroha.
We will overview commands and queries that the system has, and the set of client libraries encompassing transport and application layer logic.

Purpose of the docs you are reading
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The documentation describes the endpoints for users to send transactions to peer network, containing one or many commands to perform allowed actions in the system, and also make queries to know the current state. Also the documentation contains explanation how to build the system, how to run it and other applicable tutorials.

The API is organized around protobuf format, as Iroha is using gRPC in transport level. Iroha CLI and block store use JSON format to provide developer-friendly experience for contributors. Feel free to check both sections for protobuf and JSON while exploring docs.

API Reference
^^^^^^^^^^^^^
To try out a basic API functionality, do the following:

1. Run the system (irohad daemon) on a single node.
2. Run iroha-cli in interactive mode. Command Line Interface app is a client, showing the capabilities of the system.
3. Select a necessary action to perform. As you created an initial configuration in genesis block — use the account from genesis block to send transaction or make a query.
4. When you fill all the necessary details for commands and formed a transaction; or if you formed a query — you are ready to send it to Iroha peer. Tell the network address and port (by default it is 50051).

.. toctree::
    :maxdepth: 2
    :caption: Table of contents

    commands.rst
    queries.rst
    cpp_library.rst
    java_library.rst
    objc_library.rst
    swift_library.rst
    python_library.rst
    nodejs_library.rst