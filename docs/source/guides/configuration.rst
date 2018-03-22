Configuration
=============

In this section we will understand how to configure Iroha. Let's take a look
at ``example/config.sample``

.. code-block:: json
  :linenos:

  {
    "block_store_path": "/tmp/block_store/",
    "torii_port": 50051,
    "internal_port": 10001,
    "pg_opt": "host=localhost port=5432 user=postgres password=mysecretpassword",
    "max_proposal_size": 10,
    "proposal_delay": 5000,
    "vote_delay": 5000,
    "load_delay": 5000
  }

As you can see, configuration file is a valid ``json`` structure. Let's go 
line-by-line and understand what every parameter means.

Deployment-specific parameters
------------------------------

- ``block_store_path`` sets path to the folder where blocks are stored.
- ``torii_port`` sets the port for external communications. Queries and
  transactions are sent here.
- ``internal_port`` sets the port for internal communications: ordering
  service, consensus and block loader.
- ``pg_opt`` is used for setting credentials of PostgreSQL: hostname, port,
  username and password.

Environment-specific parameters
-------------------------------

- ``max_proposal_size`` is the maximum amount of transactions that can be in
  one proposal, and as a result in a single block as well. So, by changing this 
  value you define the size of potential block. For a starter you can stick to 
  ``10``. However, we recommend to increase this number if you have a lot of 
  transactions per second.
- ``proposal_delay`` is a maximum waiting time in milliseconds before emitting
  a new proposal. Proposal is emitted if the ``max_proposal_size`` is reached 
  or ``proposal_delay`` milliseconds had passed. You can start with ``5000``
  and increase this number if you have a lot of transactions per second since
  it is likely that with an intense load (over 100 transactions per second)
  and low value of ``proposal_delay`` there will be many proposals of small
  size.
- ``vote_delay`` is a waiting time in milliseconds before sending vote to the
  next peer. Optimal value depends heavily on the amount of Iroha peers in the
  network (higher amount of nodes requires longer ``vote_delay``). We recommend
  to start with 100-1000 milliseconds.
- ``load_delay`` is a waiting time in milliseconds before loading committed 
  block from next peer. We recommend setting this number the same value as 
  ``proposal_delay`` or even higher.
