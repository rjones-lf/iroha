FAQ
===

I'm new. Where to start?
------------------------

Hello, newcomer! You are very welcome :) \
There are 2 ways for you to start with Iroha: \
1. You can see what Iroha is an how it works by building a simple example network following our `Getting Started Guide <https://iroha.readthedocs.io/en/latest/getting_started/index.html>`_ \
2. You can get acquainted with the `core concepts of Iroha <https://iroha.readthedocs.io/en/latest/core_concepts/index.html>`_ and start building your own `Iroha network <https://iroha.readthedocs.io/en/latest/guides/index.html>`_ \\

Now you have your Iroha blockchain! Congratulations! \
If you have any questions on it, do not hesitate to contact our community here: https://chat.hyperledger.org/channel/iroha

What type of data can be transferred?
-------------------------------------

Hyperleder Iroha allows you to send not only assets (you might get such impression due to a highly developed set of commands and queries for serving such assets) but any data that will be stored in the chain as well.

The current implementation provides that opportunity at least via `SetAccountDetail <https://iroha.readthedocs.io/en/latest/api/commands.html#set-account-detail>`_ command and `GetAccountDetail <https://iroha.readthedocs.io/en/latest/api/queries.html#get-account-detail>`_ query.

Can mobile device be a node?
----------------------------

There are two options depending on what you mean by mobile device. \

If we are speaking about ARM-based hardware with some linux onboard (like Raspberry PI) or rooted Android device, then it is possible to launch Iroha as a node (a peer within a network) on that device. In that case, Iroha will run as a platform-native binary. \\

If we are speaking about default iOS or Android device with untouched factory shell (GUI), then it is generally not possible and we are not providing any instructions regarding this. Though you still can create mobile applications that use Iroha. They will be Iroha clients and would not serve as peers. \\

In order to run Iroha on ARM device you have to build it on the target platform. Building Iroha requires notable amount of RAM - for 32-bit ARM host you will need 8GB RAM. The build can be performed inside Docker container. To prepare the container you will need to: \

1. Clone Iroha git repo: https://github.com/hyperledger/iroha \
2. Do docker build -t iroha-build-env . being in iroha/docker/develop \
3. Run the newly built container and build there Iroha itself \\

Please do not forget to mount a folder with Iroha git repository to the container

What is the throughout?
-----------------------

The efficiency of your Iroha network will depend on the configuration, hardware and number of nodes. \
We currently have no set benchmarks. But we will add them soon! \\

.. note::We will be adding new frequent questions here!
