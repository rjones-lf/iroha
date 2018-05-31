***********
Permissions
***********

Hyperledger Iroha uses a role-based access control system to limit actions of its users.
This system greatly helps to implement use cases involving user groups having different access levels —
ranging from the weak users, who can't even receive asset transfer to the super-users.
The beauty of our permission system is that you don't have to have a super-user
in your Iroha setup or use all the possible permissions: you can create segregated and lightweight roles.

Maintenance of the system involves setting up roles and permissions, that are included in the roles.
This might be done at the initial step of system deployment — in genesis block,
or later when Iroha network is up and running, roles can be changed (if there is a role that can do that :)

This section will help you to understand permissions and give you an idea of how to create roles including certain permissions.

Command-related permissions
===========================

Account
-------

can_create_account
^^^^^^^^^^^^^^^^^^

Allows creating new `accounts <../core_concepts/glossary.html#account>`__.

Related API method: `Create Account <../api/commands.html#create-account>`__

can_set_detail
^^^^^^^^^^^^^^

Allows setting `account <../core_concepts/glossary.html#account>`__ detail.

.. Note:: Due to a known `issue <https://soramitsu.atlassian.net/browse/IR-1374>`__, permission does not take an effect.

Related API method: `Set Account Detail <../api/commands.html#set-account-detail>`__

can_set_my_account_detail
^^^^^^^^^^^^^^^^^^^^^^^^^

.. Hint:: This is a grantable permission.

`Permission <../core_concepts/glossary.html#permission>`__ that allows a specified `account <../core_concepts/glossary.html#account>`__ to set details for the another specified account.

See the example (to be done) for the usage details.

Related API method: `Set Account Detail <../api/commands.html#set-account-detail>`__

Asset
-----

can_create_asset
^^^^^^^^^^^^^^^^

Allows creating new `assets <../core_concepts/glossary.html#asset>`__.

Related API method: `Create Asset <../api/commands.html#create-asset>`__

can_receive
^^^^^^^^^^^

Allows `account <../core_concepts/glossary.html#account>`__ receive `assets <../core_concepts/glossary.html#asset>`__.

Related API method: `Transfer Asset <../api/commands.html#transfer-asset>`__

can_transfer
^^^^^^^^^^^^

Allows sending `assets <../core_concepts/glossary.html#asset>`__ from an `account <../core_concepts/glossary.html#account>`__ of `transaction <../core_concepts/glossary.html#transaction>`__ creator.

You can transfer an asset from one `domain <../core_concepts/glossary.html#domain>`__ to another, even if the other domain does not have an asset with the same name.

.. Note:: Destination account should have `can_receive`_ permission.

Related API method: `Transfer Asset <../api/commands.html#transfer-asset>`__

can_transfer_my_assets
^^^^^^^^^^^^^^^^^^^^^^

.. Hint:: This is a grantable permission.

`Permission <../core_concepts/glossary.html#permission>`__ that allows a specified `account <../core_concepts/glossary.html#account>`__ to transfer `assets <../core_concepts/glossary.html#asset>`__ of another specified account.

See the example (to be done) for the usage details.

Related API method: `Transfer Asset <../api/commands.html#transfer-asset>`__

Asset Quantity
--------------

can_add_asset_qty
^^^^^^^^^^^^^^^^^

Allows issuing `assets <../core_concepts/glossary.html#asset>`__.

The corresponding `command <../core_concepts/glossary.html#command>`__ can be executed only for an `account <../core_concepts/glossary.html#account>`__ of `transaction <../core_concepts/glossary.html#transaction>`__ creator and only if that account has a `role <../core_concepts/glossary.html#role>`__ with the `permission <../core_concepts/glossary.html#permission>`__.

Related API method: `Add Asset Quantity <../api/commands.html#add-asset-quantity>`__

can_subtract_asset_qty
^^^^^^^^^^^^^^^^^^^^^^

Allows burning `assets <../core_concepts/glossary.html#asset>`__.

The corresponding `command <../core_concepts/glossary.html#command>`__ can be executed only for an `account <../core_concepts/glossary.html#account>`__ of `transaction <../core_concepts/glossary.html#transaction>`__ creator and only if that account has a `role <../core_concepts/glossary.html#role>`__ with the `permission <../core_concepts/glossary.html#permission>`__.

Related API method: `Subtract Asset Quantity <../api/commands.html#subtract-asset-quantity>`__

Domain
------

can_create_domain
^^^^^^^^^^^^^^^^^

Allows creating new `domains <../core_concepts/glossary.html#domain>`__ within the system.

Related API method: `Create Domain <../api/commands.html#create-domain>`__

Grant
-----

can_grant_can_add_my_signatory
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows `role <../core_concepts/glossary.html#role>`__ owners grant `can_add_my_signatory`_ `permission <../core_concepts/glossary.html#permission>`__.

Related API method: `Grant Permission <../api/commands.html#grant-permission>`__

can_grant_can_remove_my_signatory
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows `role <../core_concepts/glossary.html#role>`__ owners grant `can_remove_my_signatory`_ `permission <../core_concepts/glossary.html#permission>`__.

Related API method: `Grant Permission <../api/commands.html#grant-permission>`__

can_grant_can_set_my_account_detail
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows `role <../core_concepts/glossary.html#role>`__ owners grant `can_set_my_account_detail`_ `permission <../core_concepts/glossary.html#permission>`__.

Related API method: `Grant Permission <../api/commands.html#grant-permission>`__

can_grant_can_set_my_quorum
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows `role <../core_concepts/glossary.html#role>`__ owners grant `can_set_my_quorum`_ `permission <../core_concepts/glossary.html#permission>`__.

Related API method: `Grant Permission <../api/commands.html#grant-permission>`__

can_grant_can_transfer_my_assets
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows `role <../core_concepts/glossary.html#role>`__ owners grant `can_transfer_my_assets`_ `permission <../core_concepts/glossary.html#permission>`__.

Related API method: `Grant Permission <../api/commands.html#grant-permission>`__

Peer
----

can_add_peer
^^^^^^^^^^^^

Allows adding `peers <../core_concepts/glossary.html#peer>`__ to the network.

A new peer will be a valid participant in the next `consensus <../core_concepts/glossary.html#consensus>`__ round after an agreement on `transaction <../core_concepts/glossary.html#transaction>`__ containing "addPeer" `command <../core_concepts/glossary.html#command>`__.

Related API method: `Add Peer <../api/commands.html#add-peer>`__

Role
----

can_append_role
^^^^^^^^^^^^^^^

Allows appending `roles <../core_concepts/glossary.html#role>`__ to another `account <../core_concepts/glossary.html#account>`__.

Related API method: `Append Role <../api/commands.html#append-role>`__

can_create_role
^^^^^^^^^^^^^^^

Allows creating a new `role <../core_concepts/glossary.html#role>`__ within a system.

Related API method: `Create Role <../api/commands.html#create-role>`__

can_detach_role
^^^^^^^^^^^^^^^

Allows revoking a `role <../core_concepts/glossary.html#role>`__ from a user.

Related API method: `Detach Role <../api/commands.html#detach-role>`__

Signatory
---------

can_add_my_signatory
^^^^^^^^^^^^^^^^^^^^

.. Hint:: This is a grantable permission.

`Permission <../core_concepts/glossary.html#permission>`__ that allows a specified `account <../core_concepts/glossary.html#account>`__ to add an extra public key to the another specified account.

Related API method: `Add Signatory <../api/commands.html#add-signatory>`__

can_add_signatory
^^^^^^^^^^^^^^^^^

Allows linking additional public keys to `account <../core_concepts/glossary.html#account>`__.

The corresponding `command <../core_concepts/glossary.html#command>`__ can be executed only for an account of `transaction <../core_concepts/glossary.html#transaction>`__ creator and only if that account has a `role <../core_concepts/glossary.html#role>`__ with the `permission <../core_concepts/glossary.html#permission>`__.

Related API method: `Add Signatory <../api/commands.html#add-signatory>`__

can_remove_my_signatory
^^^^^^^^^^^^^^^^^^^^^^^

.. Hint:: This is a grantable permission.

`Permission <../core_concepts/glossary.html#permission>`__ that allows a specified `account <../core_concepts/glossary.html#account>`__ remove public key from the another specified account.

See the example (to be done) for the usage details.

Related API method: `Remove Signatory <../api/commands.html#remove-signatory>`__

can_remove_signatory
^^^^^^^^^^^^^^^^^^^^

Allows unlinking additional public keys from an `account <../core_concepts/glossary.html#account>`__.

The corresponding `command <../core_concepts/glossary.html#command>`__ can be executed only for an account of `transaction <../core_concepts/glossary.html#transaction>`__ creator and only if that account has a `role <../core_concepts/glossary.html#role>`__ with the `permission <../core_concepts/glossary.html#permission>`__.

Related API method: `Remove Signatory <../api/commands.html#remove-signatory>`__

can_set_my_quorum
^^^^^^^^^^^^^^^^^

.. Hint:: This is a grantable permission.

`Permission <../core_concepts/glossary.html#permission>`__ that allows a specified `account <../core_concepts/glossary.html#account>`__ to set `quorum <../core_concepts/glossary.html#quorum>`__ for the another specified account.

See the example (to be done) for the usage details.

Related API method: `Set Account Quorum <../api/commands.html#set-account-quorum>`__

can_set_quorum
^^^^^^^^^^^^^^

Allows setting `quorum <../core_concepts/glossary.html#quorum>`__.

At least the same number (or more) of public keys should be already linked to an `account <../core_concepts/glossary.html#account>`__.

Related API method: `Set Account Quorum <../api/commands.html#set-account-quorum>`__

Query-related permissions
=========================

Account
-------

can_get_all_acc_detail
^^^^^^^^^^^^^^^^^^^^^^

Allows getting all the details set to any `account <../core_concepts/glossary.html#account>`__ within the system.

Related API method: To be done

can_get_all_accounts
^^^^^^^^^^^^^^^^^^^^

Allows getting `account <../core_concepts/glossary.html#account>`__ information: `quorum <../core_concepts/glossary.html#quorum>`__ and all the details related to the account.

With this `permission <../core_concepts/glossary.html#permission>`__, `query <../core_concepts/glossary.html#query>`__ creator can get information about any account within a system.

All the details (set by the account owner or owners of other accounts) will be returned.

Related API method: `Get Account <../api/queries.html#get-account>`__

can_get_domain_acc_detail
^^^^^^^^^^^^^^^^^^^^^^^^^

Allows getting all the details set to any `account <../core_concepts/glossary.html#account>`__ within the same `domain <../core_concepts/glossary.html#domain>`__ as a domain of `query <../core_concepts/glossary.html#query>`__ creator account.

Related API method: To be done

can_get_domain_accounts
^^^^^^^^^^^^^^^^^^^^^^^

Allows getting `account <../core_concepts/glossary.html#account>`__ information: `quorum <../core_concepts/glossary.html#quorum>`__ and all the details related to the account.

With this `permission <../core_concepts/glossary.html#permission>`__, `query <../core_concepts/glossary.html#query>`__ creator can get information only about accounts from the same `domain <../core_concepts/glossary.html#domain>`__.

All the details (set by the account owner or owners of other accounts) will be returned.

Related API method: `Get Account <../api/queries.html#get-account>`__

can_get_my_acc_detail
^^^^^^^^^^^^^^^^^^^^^

Allows getting all the details set to the `account <../core_concepts/glossary.html#account>`__ of `query <../core_concepts/glossary.html#query>`__ creator.

Related API method: To be done

can_get_my_account
^^^^^^^^^^^^^^^^^^

Allows getting `account <../core_concepts/glossary.html#account>`__ information: `quorum <../core_concepts/glossary.html#quorum>`__ and all the details related to the account.

With this `permission <../core_concepts/glossary.html#permission>`__, `query <../core_concepts/glossary.html#query>`__ creator can get information only about own account.

All the details (set by the account owner or owners of other accounts) will be returned.

Related API method: `Get Account <../api/queries.html#get-account>`__

Account Asset
-------------

can_get_all_acc_ast
^^^^^^^^^^^^^^^^^^^

Allows getting a balance of specified `asset <../core_concepts/glossary.html#asset>`__ on any `account <../core_concepts/glossary.html#account>`__ within the system.

Related API method: `Get Account Assets <../api/queries.html#get-account-assets>`__

can_get_domain_acc_ast
^^^^^^^^^^^^^^^^^^^^^^

Allows getting a balance of specified `asset <../core_concepts/glossary.html#asset>`__ on any `account <../core_concepts/glossary.html#account>`__ within the same `domain <../core_concepts/glossary.html#domain>`__ as a domain of `query <../core_concepts/glossary.html#query>`__ creator account.

Related API method: `Get Account Assets <../api/queries.html#get-account-assets>`__

can_get_my_acc_ast
^^^^^^^^^^^^^^^^^^

Allows getting a balance of specified `asset <../core_concepts/glossary.html#asset>`__ on `account <../core_concepts/glossary.html#account>`__ of `query <../core_concepts/glossary.html#query>`__ creator.

Related API method: `Get Account Assets <../api/queries.html#get-account-assets>`__

Account Asset Transaction
-------------------------

can_get_all_acc_ast_txs
^^^^^^^^^^^^^^^^^^^^^^^

Allows getting `transactions <../core_concepts/glossary.html#transaction>`__ associated with a specified `asset <../core_concepts/glossary.html#asset>`__ and any `account <../core_concepts/glossary.html#account>`__ within the system.

.. Note:: Incoming asset transfers will also appear in the command output.

Related API method: `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

can_get_domain_acc_ast_txs
^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows getting `transactions <../core_concepts/glossary.html#transaction>`__ associated with a specified `asset <../core_concepts/glossary.html#asset>`__ and an `account <../core_concepts/glossary.html#account>`__ from the same `domain <../core_concepts/glossary.html#domain>`__ as `query <../core_concepts/glossary.html#query>`__ creator.

.. Note:: Incoming asset transfers will also appear in the command output.

Related API method: `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

can_get_my_acc_ast_txs
^^^^^^^^^^^^^^^^^^^^^^

Allows getting `transactions <../core_concepts/glossary.html#transaction>`__ associated with the `account <../core_concepts/glossary.html#account>`__ of `query <../core_concepts/glossary.html#query>`__ creator and specified `asset <../core_concepts/glossary.html#asset>`__.

.. Note:: Incoming asset transfers will also appear in the command output.

Related API method: `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

Account Transaction
-------------------

can_get_all_acc_txs
^^^^^^^^^^^^^^^^^^^

Allows getting all `transactions <../core_concepts/glossary.html#transaction>`__ issued by any `account <../core_concepts/glossary.html#account>`__ within the system.

.. Note:: Incoming asset transfer inside a transaction would NOT lead to an appearance of the transaction in the command output.

Related API method: `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

can_get_domain_acc_txs
^^^^^^^^^^^^^^^^^^^^^^

Allows getting all `transactions <../core_concepts/glossary.html#transaction>`__ issued by any `account <../core_concepts/glossary.html#account>`__ from the same `domain <../core_concepts/glossary.html#domain>`__ as `query <../core_concepts/glossary.html#query>`__ creator.

.. Note:: Incoming asset transfer inside a transaction would NOT lead to an appearance of the transaction in the command output.

Related API method: `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

can_get_my_acc_txs
^^^^^^^^^^^^^^^^^^

Allows getting all `transactions <../core_concepts/glossary.html#transaction>`__ issued by an `account <../core_concepts/glossary.html#account>`__ of `query <../core_concepts/glossary.html#query>`__ creator.

.. Note:: Incoming asset transfer inside a transaction would NOT lead to an appearance of the transaction in the command output.

Related API method: `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

Asset
-----

can_read_assets
^^^^^^^^^^^^^^^

Allows getting information about `asset <../core_concepts/glossary.html#asset>`__ precision.

Related API method: `Get Asset Info <../api/queries.html#get-asset-info>`__

Block Stream
------------

can_get_blocks
^^^^^^^^^^^^^^

Not implemented now. Allows subscription to the stream of accepted `blocks <../core_concepts/glossary.html#block>`__.

Role
----

can_get_roles
^^^^^^^^^^^^^

Allows getting a list of `roles <../core_concepts/glossary.html#role>`__ within the system.
Allows getting a list of `permissions <../core_concepts/glossary.html#permission>`__ associated with a role.

Related API methods: `Get Roles <../api/queries.html#get-roles>`__, `Get Role Permissions <../api/queries.html#get-role-permissions>`__

Signatory
---------

can_get_all_signatories
^^^^^^^^^^^^^^^^^^^^^^^

Allows getting a list of public keys linked to an `account <../core_concepts/glossary.html#account>`__ within the system.

Related API method: `Get Signatories <../api/queries.html#get-signatories>`__

can_get_domain_signatories
^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows getting a list of public keys of any `account <../core_concepts/glossary.html#account>`__ within the same `domain <../core_concepts/glossary.html#domain>`__ as the domain of `query <../core_concepts/glossary.html#query>`__ creator account.

Related API method: `Get Signatories <../api/queries.html#get-signatories>`__

can_get_my_signatories
^^^^^^^^^^^^^^^^^^^^^^

Allows getting a list of public keys of `query <../core_concepts/glossary.html#query>`__ creator `account <../core_concepts/glossary.html#account>`__.

Related API method: `Get Signatories <../api/queries.html#get-signatories>`__

Transaction
-----------

can_get_all_txs
^^^^^^^^^^^^^^^

Allows getting any `transaction <../core_concepts/glossary.html#transaction>`__ by hash.

Related API method: `Get Transactions <../api/queries.html#get-transactions>`__

can_get_my_txs
^^^^^^^^^^^^^^

Allows getting `transaction <../core_concepts/glossary.html#transaction>`__ (that was issued by `query <../core_concepts/glossary.html#query>`__ creator) by hash.

Related API method: `Get Transactions <../api/queries.html#get-transactions>`__
