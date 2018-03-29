Commands
========

A command changes the state, called World State View, by performing an action over an entity (asset, account) in the system.
Any command should be included in a transaction to perform an action.

Add asset quantity
------------------

Purpose
^^^^^^^

The purpose of add asset quantity command is to increase the quantity of an asset on account of transaction creator.
Use case scenario is to increase the number of a mutable asset in the system, which can act as a claim on a commodity (e.g. money, gold, etc.)

Schema
^^^^^^

.. code-block:: proto

    message AddAssetQuantity {
    string account_id = 1;
    string asset_id = 2;
    Amount amount = 3;
    }

    message uint256 {
       uint64 first = 1;
       uint64 second = 2;
       uint64 third = 3;
       uint64 fourth = 4;
    }

    message Amount {
       uint256 value = 1;
       uint32 precision = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "account id in which to add asset", "account_name@domain", "alex@morgan"
    "Asset ID", "id of the asset", "asset#domain", "usd#morgan"
    "Amount", "positive amount of the asset to add", "> 0", "200.02"

Validation
^^^^^^^^^^

1. Asset and account should exist
2. Added quantity precision should be equal to asset precision
3. Creator of a transaction should have a role which has permissions for issuing assets
4. Creator of a transaction adds account quantity to his/her account only

Add peer
--------

Purpose
^^^^^^^

The purpose of add peer command is to write into ledger the fact of peer addition into the peer network.
After a transaction with AddPeer has been committed, consensus and synchronization components will start using it.

Schema
^^^^^^

.. code-block:: proto

    message AddPeer {
        Peer peer = 1;
    }

    message Peer {
        string address = 1;
        bytes peer_key = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 10, 30

    "Address", "resolvable address in network (IPv4, IPv6, domain name, etc.)", "should be resolvable", "192.168.1.1:50541"
    "Peer key", "peer public key, which is used in consensus algorithm to sign-off vote, commit, reject messages", "ed25519 public key", "292a8714694095edce6be799398ed5d6244cd7be37eb813106b217d850d261f2"

Validation
^^^^^^^^^^

1. Creator of the transaction has a role which has CanAddPeer permission
2. Such network address has not been already added

Add signatory
-------------

Purpose
^^^^^^^

The purpose of add signatory command is to add an identifier to the account.
Such identifier is a public key of another device or a public key of another user.

Schema
^^^^^^

.. code-block:: proto

    message AddSignatory {
        string account_id = 1;
        bytes public_key = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "Account to which to add signatory", "account_name@domain", "makoto@soramitsu"
    "Public key", "Signatory to add to account", "ed25519 public key", "359f925e4eeecfdd6aa1abc0b79a6a121a5dd63bb612b603247ea4f8ad160156"

Validation
^^^^^^^^^^

Two cases:

    Case 1. Transaction creator wants to add a signatory to his or her account, having permission CanAddSignatory

    Case 2. CanAddSignatory was granted to transaction creator

Append role
-----------

Purpose
^^^^^^^

The purpose of append role command is to promote an account to some created role in the system, where a role is a set of permissions account has to perform an action (command or query).

Schema
^^^^^^

.. code-block:: proto

    message AppendRole {
       string account_id = 1;
       string role_name = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "id or account to append role to", "already existent", "makoto@soramitsu"
    "Role name", "name of already created role", "already existent", "MoneyCreator"

Validation
^^^^^^^^^^

1. The role should exist in the system
2. Transaction creator should have permissions to append role (CanAppendRole)
3. Account, which appends role, has set of permissions in his roles that is a superset of appended role (in other words no-one can append role that is more powerful than what transaction creator is)

Create account
--------------

Purpose
^^^^^^^

The purpose of create account command is to make entity in the system, capable of sending transactions or queries, storing signatories, personal data and identifiers.

Schema
^^^^^^

.. code-block:: proto

    message CreateAccount {
        string account_name = 1;
        string domain_id = 2;
        bytes main_pubkey = 3;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account name", "domain-unique name for account", "A string in domain-name syntax defined in RFC1035 [#f1]_ ", "morgan.stanley"
    "Domain ID", "target domain to make relation with", "should be created before the account", "america"
    "Main pubkey", "first public key to add to the account", "ed25519 public key", "407e57f50ca48969b08ba948171bb2435e035d82cec417e18e4a38f5fb113f83"

Validation
^^^^^^^^^^

1. Transaction creator has permission to create an account
2. Domain, passed as domain_id, has already been created in the system
3. Such public key has not been added before as first public key of account or added to a multi-signature account

.. [#f1] https://www.ietf.org/rfc/rfc1035.txt

Create asset
------------

Purpose
^^^^^^^

The purpose of сreate asset command is to create a new type of asset, unique in a domain.
An asset is a countable representation of a commodity.

Schema
^^^^^^

.. code-block:: proto

    message CreateAsset {
        string asset_name = 1;
        string domain_id = 2;
        uint32 precision = 3;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Asset name", "domain-unique name for asset", "`[A-Za-z0-9]{1,9}`", "soracoin"
    "Domain ID", "target domain to make relation with", "already existent", "japan"
    "Precision", "number of digits after comma/dot", "0 <= precision <= uint32 max", "2"

Validation
^^^^^^^^^^

1. Transaction creator has permission to create assets
2. Asset name is unique in domain

Create domain
-------------

Purpose
^^^^^^^

The purpose of create domain command is to make new domain in Iroha network, which is a group of accounts.

Schema
^^^^^^

.. code-block:: proto

    message CreateDomain {
        string domain_id = 1;
        string default_role = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Domain ID", "ID for created domain", "unique, `[0-9A-Za-z]{1,9}`", "japan05"
    "Default role", "role for any created user in the domain", "one of the existing roles", "User"

Validation
^^^^^^^^^^

1. Domain ID is unique
2. Account, who sends this command in transaction, has role with permission to create domain
3. Role, which will be assigned to created user by default, exists in the system

Create role
-----------

Purpose
^^^^^^^

The purpose of create role command is to create a new role in the system from the set of permissions.
Combining different permissions into roles, maintainers of Iroha peer network can create customized security model.

Schema
^^^^^^

.. code-block:: proto

    message CreateRole {
       string role_name = 1;
       repeated string permissions = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Role name", "name of role to create", "`[A-Za-z0-9_]{1,7}`", "User"
    "Permissions", "array of already existent permissions", "set of passed permissions is fully included into set of existing permissions", "{can_receive, can_transfer}"

Validation
^^^^^^^^^^

1. Set of passed permissions is fully included into set of existing permissions
2. Set of the permissions is not empty

Detach role
-----------

Purpose
^^^^^^^

The purpose of detach role command is to detach a role from the set of roles of an account.
By executing this command it is possible to decrease the number of possible actions in the system for the user.

Schema
^^^^^^

.. code-block:: proto

    message DetachRole {
        string account_id = 1;
        string role_name = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "ID of account where role will be deleted", "already existent", "makoto@soramitsu"
    "Role name", "a detached role name", "existing role", "User"

Validation
^^^^^^^^^^

1. The role exists in the system
2. The account has such role

Grant permission
----------------

Purpose
^^^^^^^

The purpose of grant permission command is to give another account rights to perform actions on the account of transaction sender (give someone right to do something with my account).

Schema
^^^^^^

.. code-block:: proto

    message GrantPermission {
       string account_id = 1;
       string permission_name = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "id of account whom rights are granted", "already existent", "makoto@soramitsu"
    "Permission name", "name of granted permission", "permission is defined", "CanTransferAssets"


Validation
^^^^^^^^^^

1. Account exists
2. Transaction creator is allowed to grant this permission

Remove signatory
----------------

Purpose
^^^^^^^

Purpose of remove signatory command is to remove a public key, associated with an identity, from an account

Schema
^^^^^^

.. code-block:: proto

    message RemoveSignatory {
        string account_id = 1;
        bytes public_key = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "id of account whom rights are granted", "already existent", "makoto@soramitsu"
    "Public key", "Signatory to delete", "ed25519 public key", "407e57f50ca48969b08ba948171bb2435e035d82cec417e18e4a38f5fb113f83"

Validation
^^^^^^^^^^

1. When signatory is deleted, we should check if invariant of **size(signatories) >= quorum** holds
2. Signatory should have been previously added to the account

Two cases:

    Case 1. When transaction creator wants to remove signatory from their account and he or she has permission CanRemoveSignatory

    Case 2. CanRemoveSignatory was granted to transaction creator

Revoke permission
-----------------

Purpose
^^^^^^^

The purpose of revoke permission command is to revoke or dismiss given granted permission from another account in the network.

Schema
^^^^^^

.. code-block:: proto

    message RevokePermission {
       string account_id = 1;
       string permission_name = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

        "Account ID", "id of account whom rights are granted", "already existent", "makoto@soramitsu"
        "Permission name", "name of granted permission", "permission was granted", "CanTransferAssets"

Validation
^^^^^^^^^^

Transaction creator should have previously granted this permission to a target account

Set account detail
------------------

Purpose
^^^^^^^

Purpose of set account detail command is to set key-value information for a given account

Schema
^^^^^^

.. code-block:: proto

    message SetAccountDetail{
        string account_id = 1;
        string key = 2;
        string value = 3;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "id of account whom key-value information was set", "already existent", "makoto@soramitsu"
    "Key", "key of information being set", "`[A-Za-z0-9_]{1,}`", "Name"
    "Value", "value of corresponding key", "None", "Makoto"

Validation
^^^^^^^^^^

Two cases:

    Case 1. When transaction creator wants to set account detail to his/her account and he or she has permission CanSetAccountInfo

    Case 2. CanSetAccountInfo was granted to transaction creator

Set account quorum
------------------

Purpose
^^^^^^^

The purpose of set account quorum command is to set the number of signatories required to confirm the identity of a user, who creates the transaction.
Use case scenario is to set the number of different users, utilizing single account, to sign off the transaction.

Schema
^^^^^^

.. code-block:: proto

    message SetAccountQuorum {
        string account_id = 1;
        uint32 quorum = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "ID of account to set quorum", "already existent", "makoto@soramitsu"
    "Quorum", "number of signatories needed to be included with a transaction from this account", "0 < quorum < 10", "5"

Validation
^^^^^^^^^^

When quorum is set, it is checked if invariant of **size(signatories) >= quorum** holds.

Two cases:

    Case 1. When transaction creator wants to set quorum for his/her account and he or she has permission CanRemoveSignatory

    Case 2. CanRemoveSignatory was granted to transaction creator

Subtract asset quantity
-----------------------

Purpose
^^^^^^^

The purpose of subtract asset quantity command is the opposite of AddAssetQuantity commands — to decrease the number of assets on account of transaction creator.

Schema
^^^^^^

.. code-block:: proto

    message AddAssetQuantity {
        string account_id = 1;
        string asset_id = 2;
        Amount amount = 3;
    }

    message uint256 {
       uint64 first = 1;
       uint64 second = 2;
       uint64 third = 3;
       uint64 fourth = 4;
    }

    message Amount {
       uint256 value = 1;
       uint32 precision = 2;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "account id from which to subtract asset", "already existent", "makoto@soramitsu"
    "Asset ID", "id of the asset", "asset#domain", "usd#morgan"
    "Amount", "positive amount of the asset to subtract", "> 0", "200"

Validation
^^^^^^^^^^

1. Asset and account should exist
2. Added quantity precision should be equal to asset precision
3. Creator of the transaction should have a role which has permissions for subtraction of assets
4. Creator of transaction subtracts account quantity in his/her account only

Transfer asset
--------------

Purpose
^^^^^^^

The purpose of transfer asset command is to share assets within the account in peer network: in the way that source account transfers assets to the target account.

Schema
^^^^^^

.. code-block:: proto

    message TransferAsset {
        string src_account_id = 1;
        string dest_account_id = 2;
        string asset_id = 3;
        string description = 4;
        Amount amount = 5;
    }

Structure
^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Source account ID", "ID of account to withdraw asset from", "already existent", "makoto@soramitsu"
    "Destination account ID", "ID of account to send asset at", "already existent", "alex@california"
    "Asset ID", "ID of asset to transfer", "already existent", "usd#usa"
    "Description", "Message to attach to transfer", "No constraints", "here's my money take it"
    "Amount", "amount of the asset to transfer", "0 < amount < max_uint256", "200.20"

Validation
^^^^^^^^^^

1. Source account has this asset in its AccountHasAsset relation
2. An amount is a positive number and asset precision is consistent with the asset definition
3. Source account has enough amount of asset to transfer and is not zero
4. Source account can transfer money, and destination account can receive money (their roles have these permissions)
