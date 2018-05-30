***********
Permissions
***********

Command-related permissions
===========================

Account
-------

can_create_account
^^^^^^^^^^^^^^^^^^

Allows creating new accounts.

Related API method
   `Create Account <../api/commands.html#create-account>`__

can_set_detail
^^^^^^^^^^^^^^

Allows setting account detail.

Permission does not take an effect.

Related API method
   `Set Account Detail <../api/commands.html#set-account-detail>`__

can_set_my_account_detail
^^^^^^^^^^^^^^^^^^^^^^^^^

.. Hint:: This is grantable permission.

Permission that allows a specified account to set details for the another specified account.

Please see the example for the usage details.

Related API method
   `Set Account Detail <../api/commands.html#set-account-detail>`__

Asset
-----

can_create_asset
^^^^^^^^^^^^^^^^

Allows creating new assets.

Related API method
   `Create Asset <../api/commands.html#create-asset>`__

can_receive
^^^^^^^^^^^

Allows for account receive assets.

Related API method
   `Transfer Asset <../api/commands.html#transfer-asset>`__

can_transfer
^^^^^^^^^^^^

Allows sending assets from an account of transaction creator.

Please note that destination account should have `can_receive`_ permission.

You can transfer an asset from one domain to another, even if the other domain does not have an asset with the same name.

Related API method
   `Transfer Asset <../api/commands.html#transfer-asset>`__

can_transfer_my_assets
^^^^^^^^^^^^^^^^^^^^^^

.. Hint:: This is grantable permission.

Permission that allows a specified account to transfer assets of another specified account.

Please see the example for the usage details.

Related API method
   `Transfer Asset <../api/commands.html#transfer-asset>`__

Asset Quantity
--------------

can_add_asset_qty
^^^^^^^^^^^^^^^^^

Allows issuing assets.

The corresponding command can be executed only for an account of transaction creator and only if that account has a role with the permission.

Related API method
   `Add Asset Quantity <../api/commands.html#add-asset-quantity>`__

can_subtract_asset_qty
^^^^^^^^^^^^^^^^^^^^^^

Allows burning assets.

The corresponding command can be executed only for an account of transaction creator and only if that account has a role with the permission.

Related API method
   `Subtract Asset Quantity <../api/commands.html#subtract-asset-quantity>`__

Domain
------

can_create_domain
^^^^^^^^^^^^^^^^^

Allows creating new domains within the system.

Related API method
   `Create Domain <../api/commands.html#create-domain>`__

Grant
-----

can_grant_can_add_my_signatory
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows to role owners grant `can_add_my_signatory`_ permission.

Related API method
   `Grant Permission <../api/commands.html#grant-permission>`__

can_grant_can_remove_my_signatory
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows to role owners grant `can_remove_my_signatory`_ permission.

Related API method
   `Grant Permission <../api/commands.html#grant-permission>`__

can_grant_can_set_my_account_detail
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows to role owners grant `can_set_my_account_detail`_ permission.

Related API method
   `Grant Permission <../api/commands.html#grant-permission>`__

can_grant_can_set_my_quorum
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows to role owners grant `can_set_my_quorum`_ permission.

Related API method
   `Grant Permission <../api/commands.html#grant-permission>`__

can_grant_can_transfer_my_assets
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows to role owners grant `can_transfer_my_assets`_ permission.

Related API method
   `Grant Permission <../api/commands.html#grant-permission>`__

Peer
----

can_add_peer
^^^^^^^^^^^^

Allows adding peers to the network.

Related API method
   `Add Peer <../api/commands.html#add-peer>`__

Role
----

can_append_role
^^^^^^^^^^^^^^^

Allows appending roles to another account.

Please check the glossary for the detailed description of roles and permissions.

Related API method
   `Append Role <../api/commands.html#append-role>`__

can_create_role
^^^^^^^^^^^^^^^

Allows creating a new role with a specified set of permissions within a system.

Related API method
   `Create Role <../api/commands.html#create-role>`__

can_detach_role
^^^^^^^^^^^^^^^

Allows revoking a named set of permissions (a role) from a user.

Related API method
   `Detach Role <../api/commands.html#detach-role>`__

Signatory
---------

can_add_my_signatory
^^^^^^^^^^^^^^^^^^^^

.. Hint:: This is grantable permission.

Permission that allows a specified account to add an extra public key to the another specified account.

Related API method
   `Add Signatory <../api/commands.html#add-signatory>`__

can_add_signatory
^^^^^^^^^^^^^^^^^

Allows linking additional public keys to account.

The corresponding command can be executed only for an account of transaction creator and only if that account has a role with the permission.

Related API method
   `Add Signatory <../api/commands.html#add-signatory>`__

can_remove_my_signatory
^^^^^^^^^^^^^^^^^^^^^^^

.. Hint:: This is grantable permission.

Permission that allows a specified account remove public key from the another specified account.

Please see the example for the usage details.

Related API method
   `Remove Signatory <../api/commands.html#remove-signatory>`__

can_remove_signatory
^^^^^^^^^^^^^^^^^^^^

Allows unlinking additional public keys from an account.

The corresponding command can be executed only for an account of transaction creator and only if that account has a role with the permission.

Related API method
   `Remove Signatory <../api/commands.html#remove-signatory>`__

can_set_my_quorum
^^^^^^^^^^^^^^^^^

.. Hint:: This is grantable permission.

Permission that allows a specified account to set quorum number for the another specified account.

Please see the example for the usage details.

Related API method
   `Set Account Quorum <../api/commands.html#set-account-quorum>`__

can_set_quorum
^^^^^^^^^^^^^^

Allows setting a minimum number of signatures required for transaction signing.

At least the same number (or more) of public keys should be already linked to an account.

Related API method
   `Set Account Quorum <../api/commands.html#set-account-quorum>`__

Query-related permissions
=========================

Account
-------

can_get_all_acc_detail
^^^^^^^^^^^^^^^^^^^^^^

Allows getting all the details set to any account within the system.

Related API method
   To be done

can_get_all_accounts
^^^^^^^^^^^^^^^^^^^^

Allows getting account information: quorum and all the details related to the account.

With this permission, query creator can get information about any account within a system.

All the details (set by the account owner or owners of other accounts) will be returned.

Related API method
   `Get Account <../api/queries.html#get-account>`__

can_get_domain_acc_detail
^^^^^^^^^^^^^^^^^^^^^^^^^

Allows getting all the details set to any account within the same domain as a domain of query creator account.

Related API method
   To be done

can_get_domain_accounts
^^^^^^^^^^^^^^^^^^^^^^^

Allows getting account information: quorum and all the details related to the account.

With this permission, query creator can get information only about accounts from the same domain.

All the details (set by the account owner or owners of other accounts) will be returned.

Related API method
   `Get Account <../api/queries.html#get-account>`__

can_get_my_acc_detail
^^^^^^^^^^^^^^^^^^^^^

Allows getting all the details set to the account of query creator.

Related API method
   To be done

can_get_my_account
^^^^^^^^^^^^^^^^^^

Allows getting account information: quorum and all the details related to the account.

With this permission, query creator can get information only about own account.

All the details (set by the account owner or owners of other accounts) will be returned.

Related API method
   `Get Account <../api/queries.html#get-account>`__

Account Asset
-------------

can_get_all_acc_ast
^^^^^^^^^^^^^^^^^^^

Allows getting a balance of specified asset on any account within the system.

Related API method
   `Get Account Assets <../api/queries.html#get-account-assets>`__

can_get_domain_acc_ast
^^^^^^^^^^^^^^^^^^^^^^

Allows getting a balance of specified asset on any account within the same domain as a domain of query creator account.

Related API method
   `Get Account Assets <../api/queries.html#get-account-assets>`__

can_get_my_acc_ast
^^^^^^^^^^^^^^^^^^

Allows getting a balance of specified asset on account of query creator.

Related API method
   `Get Account Assets <../api/queries.html#get-account-assets>`__

Account Asset Transaction
-------------------------

can_get_all_acc_ast_txs
^^^^^^^^^^^^^^^^^^^^^^^

Allows getting transactions associated with a specified asset and any account within the system.

Related API method
   `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

can_get_domain_acc_ast_txs
^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows getting transactions associated with a specified asset and an account from the same domain as query creator.

Related API method
   `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

can_get_my_acc_ast_txs
^^^^^^^^^^^^^^^^^^^^^^

Allows getting transactions associated with the account of query creator and specified asset.

Related API method
   `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

Account Transaction
-------------------

can_get_all_acc_txs
^^^^^^^^^^^^^^^^^^^

Allows getting all transactions issued by any account within the system.

Incoming asset transfer inside a transaction would not lead to an appearance of the transaction in the command output.

Related API method
   `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

can_get_domain_acc_txs
^^^^^^^^^^^^^^^^^^^^^^

Allows getting all transactions issued by any account from the same domain as query creator.

Incoming asset transfer inside a transaction would not lead to an appearance of the transaction in the command output.

Related API method
   `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

can_get_my_acc_txs
^^^^^^^^^^^^^^^^^^

Allows getting all transactions issued by an account of query creator.

Incoming asset transfer inside a transaction would not lead to an appearance of the transaction in the command output.

Related API method
   `Get Account Asset Transactions <../api/queries.html#get-account-asset-transactions>`__

Asset
-----

can_read_assets
^^^^^^^^^^^^^^^

Allows getting information about asset precision.

Related API method
   `Get Asset Info <../api/queries.html#get-asset-info>`__

Block Stream
------------

can_get_blocks
^^^^^^^^^^^^^^

Not implemented now. Allows subscription to the stream of accepted blocks.

Role
----

can_get_roles
^^^^^^^^^^^^^

Allows getting a list of roles within the system.
Allows getting a list of permissions associated with a role.

Related API methods
   `Get Roles <../api/queries.html#get-roles>`__, `Get Role Permissions <../api/queries.html#get-role-permissions>`__

Signatory
---------

can_get_all_signatories
^^^^^^^^^^^^^^^^^^^^^^^

Allows getting a list of public keys linked to an account within the system.

Related API method
   `Get Signatories <../api/queries.html#get-signatories>`__

can_get_domain_signatories
^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows getting a list of public keys of any account within the same domain as the domain of query creator account.

Related API method
   `Get Signatories <../api/queries.html#get-signatories>`__

can_get_my_signatories
^^^^^^^^^^^^^^^^^^^^^^

Allows getting a list of public keys of query creator account.

Related API method
   `Get Signatories <../api/queries.html#get-signatories>`__

Transaction
-----------

can_get_all_txs
^^^^^^^^^^^^^^^

Allows getting any transaction by hash.

Related API method
   `Get Transactions <../api/queries.html#get-transactions>`__

can_get_my_txs
^^^^^^^^^^^^^^

Allows getting transaction (that was issued by query creator) by hash.

Related API method
   `Get Transactions <../api/queries.html#get-transactions>`__
