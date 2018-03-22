Configuration
=============
In this section we will understand how to configure Iroha. Let's take a look 
at ``example/config.sample`` and try to understand whate evrey parameter 
means:

.. code-block:: json
  :linenos:

  {
    "block_store_path" : "/tmp/block_store/",
    "torii_port" : 50051,
    "internal_port" : 10001,
    "pg_opt" : "host=localhost port=5432 user=postgres password=mysecretpassword",
    "max_proposal_size" : 10,
    "proposal_delay" : 5000,
    "vote_delay" : 5000,
    "load_delay" : 5000
  }

As you can see, configuration file is a valid ``json`` structure. Let's go 
line-by-line and understand what every parameter means.