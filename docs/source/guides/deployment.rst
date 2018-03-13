Deploying Iroha
===============

Hyperledger Iroha can be deployed in different ways, depending on the perspective and the purpose.
There can be either a single node deployed, or multiple nodes running in several containers on a local machine or spread across the network — so pick any case you need.
This page describes different scenarios and is intended to act as a how-to guide for users, primarily trying out Iroha for the first time.

Running single instance  
^^^^^^^^^^^^^^^^^^^^^^^

Generally, people want to run Iroha locally in order to try out the API and explore the capabilities.
This can be done in local or container environment (Docker).
We will explore both possible cases,
but in order to simplify peer components deployment, *it is advised to have Docker installed on your machine*.

Local environment
-----------------

By local environment, it is meant to have daemon process and Postgres deployed without any containers.
This might be helpful in cases when messing up with Docker is not preferred — generally a quick exploration of the features.

Run postgres server
"""""""""""""""""""

In order to run postgres server locally, you should check postgres `website <https://www.postgresql.org/docs/current/static/server-start.html>`__ and follow their description.
Generally, postgres server runs automatically when the system starts, but this should be checked in the configuration of the system.


Run iroha daemon (irohad)
"""""""""""""""""""""""""

There is a list of preconditions which you should meet before proceeding:

 * Postgres server is up and running
 * `irohad` Iroha daemon binary is built and accessible in your system
 * The genesis block and configuration files were created
 * Config file uses valid postgres connection settings
 * A keypair for the peer is generated
 * This is the first time you run the Iroha on this peer and you want to create new chain

.. Hint:: Have you got something that is not the same as in the list of assumptions? Please, refer to the section below the document, titled as `Dealing with troubles`_.

In case of valid assumptions, the only thing that remains is to launch the daemon process with following parameters:

+---------------+-----------------------------------------------------------------+
| Parameter     | Meaning                                                         |
+---------------+-----------------------------------------------------------------+
| config        | configuration file, containing postgres connection and values   |
|               | to tune the system                                              |
+---------------+-----------------------------------------------------------------+
| genesis_block | initial block in the ledger                                     |
+---------------+-----------------------------------------------------------------+
| keypair_name  | private and public key file names without file extension,       |
|               | used by peer to sign the blocks                                 |
+---------------+-----------------------------------------------------------------+

An example of shell command, running Iroha daemon is 

.. code-block:: shell

    irohad --config example/config.sample --genesis_block example/genesis.block --keypair_name example/node0

.. Attention:: If you have stopped the daemon and want to use existing chain — you should not pass the genesis block parameter.


Docker
------

In order to run Iroha peer as a single instance in Docker, you should pull the image for Iroha first:

.. code-block:: shell

    docker pull hyperledger/iroha-docker:latest

.. Hint:: Use *latest* tag for latest stable release, and *develop* for latest development version  

Then, you have to create an enviroment for the image to run without problems:

Create docker network
"""""""""""""""""""""

Containers for Postgres and Iroha should run in the same virtual network, in order to be available to each other.
Create a network, by typing following command (you can use any name for the network, but in the example, we use *iroha-network* name):

.. code-block:: shell

    docker network create iroha-network

Run Postgresql in a container
"""""""""""""""""""""""""""""

Similarly, run postgres server, attaching it to the network you have created before, and exposing ports for communication:

.. code-block:: shell

    docker run --name some-postgres \
    -e POSTGRES_USER=postgres \
    -e POSTGRES_PASSWORD=mysecretpassword \
    -p 5432:5432 \
    --network=iroha-network \
    -d postgres:9.5

Create volume for block storage
"""""""""""""""""""""""""""""""

Before we run iroha daemon in the container, we should create persistent volume to store files, storing blocks for the chain.
It is done via the following command:

.. code-block:: shell

    docker volume create blockstore

Running iroha daemon in docker container
""""""""""""""""""""""""""""""""""""""""

There is a list of assumptions which you should review before proceeding:
 * Postgres server is running on the same docker network
 * There is a folder, containing config file and keypair for a single node
 * This is the first time you run the Iroha on this peer and you want to create new chain

If they are met, you can move forward with the following command:

.. code-block:: shell

    docker run --name iroha \
    # External port
    -p 50051:50051 \
    # Folder with configuration files
    -v ~/Developer/iroha/example:/opt/iroha_data \
    # Blockstore volume
    -v blockstore:/tmp/block_store \
    # Postgres settings
    -e POSTGRES_HOST='some-postgres' \
    -e POSTGRES_PORT='5432' \
    -e POSTGRES_PASSWORD='mysecretpassword' \
    -e POSTGRES_USER='postgres' \
    # Node keypair name
    -e KEY='node0' \
    # Docker network name
    --network=iroha-network \
    hyperledger/iroha-docker:latest

Running multiple instances (peer network)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to set up a peer network, one should follow routines, described in this section.
In this version, we support manual deployment and automated by Ansible Playbook.
Choose an option, that meets your security criteria and other needs.

Manually
--------

By manual deployment, we mean that Iroha peer network is set up without automated assistance.
It is similar to the process of running a single local instance, although the difference is the genesis block includes more than a single peer.
In order to form a block, which includes more than a single peer, or requires customization for your needs, please take a look at `Dealing with troubles`_ section.

Automated
---------

Anyone can reuse existing Ansible Playbook in order to create a network of peers running Iroha.
Currently, this is a solution for development and testing, in other words, a proof of concept, and cannot be used in production environment, due to some security flaws.
For production network, a manual composing of genesis block is required.

Prerequisites
"""""""""""""

 * One ore more <virtual> machines with a Linux distributive installed.
 * SSH access to those machines
 * Ansible installed on a local machine

Step-by-step guide
""""""""""""""""""

1. Create peers.list file in $IROHA_HOME/deploy/ansible/data

2. Write all peers IP addresses followed by the internal port 10001 (e.g 31.192.120.36:10001)

3. Open $IROHA_HOME/deploy/ansible/hosts file

4. Write all IP addresses in [hosts] group

5. Open terminal 

6. Disable host key checking, because it can cause troubles due to interactive prompt
 
.. code-block:: shell

    export ANSIBLE_HOST_KEY_CHECKING=False

7. Go to ansible folder

.. code-block:: shell

    cd $IROHA_HOME/deploy/ansible

8. Run playbook, providing your private key and hosts file
 
.. code-block:: shell

    ansible-playbook --private-key=~/.ssh/iroha -i hosts provisioning.yml

9. Wait until playbook finishes and then Iroha network is ready and up.

Checking Iroha peer status
""""""""""""""""""""""""""

1. SSH into any of your machines
 
.. code-block:: shell

    ssh -i ~/.ssh/iroha iroha@35.205.142.238

2. Check Iroha container logs:
 
.. code-block:: shell

    docker logs iroha 

Dealing with troubles
^^^^^^^^^^^^^^^^^^^^^

—"Please, help me, because I am…"

Not having Iroha daemon binary
------------------------------

You can build Iroha daemon binary from sources, following API section from the `website <https://hyperledger.github.io/iroha-api/#build>`__

Not having config file
----------------------

Check how to create a configuration file by following this `link <https://hyperledger.github.io/iroha-api/#prepare-configuration-file>`__

Not having genesis block
------------------------

Create genesis block by generating it via `iroha-cli` or manually, as it is described `here <https://hyperledger.github.io/iroha-api/#create-genesis-block>`__

Not having a keypair for a peer
-------------------------------

In order to create a keypair for an account or a peer, use iroha-cli binary by passing the name of the peer with `--new_account` option.
