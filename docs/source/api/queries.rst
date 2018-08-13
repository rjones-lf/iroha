Queries
=======

A query is a request related to certain part of World State View — the latest state of blockchain.
Query cannot modify the contents of the chain and a response is returned
to any client immediately after receiving peer has processed a query.

Validation
^^^^^^^^^^

The validation for all queries includes:

- timestamp — shouldn't be from the past (24 hours prior to the peer time) or from the future (range of 5 minutes added to the peer time)
- signature of query creator — used for checking the identity of query creator
- query counter — checked to be incremented with every subsequent query from query creator
- roles — depending on the query creator's role: the range of state available to query can relate to to the same account, account in the domain, to the whole chain, or not allowed at all

Get Account
^^^^^^^^^^^

Purpose
-------

Purpose of get account query is to get the state of an account.

Request Schema
--------------

.. code-block:: proto

    message GetAccount {
        string account_id = 1;
    }

Request Structure
-----------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "account id to request its state", "<account_name>@<domain_id>", "alex@morgan"

Response Schema
---------------

.. code-block:: proto

    message AccountResponse {
        Account account = 1;
        repeated string account_roles = 2;
    }

    message Account {
        string account_id = 1;
        string domain_id = 2;
        uint32 quorum = 3;
        string json_data = 4;
    }


Response Structure
------------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "account id", "<account_name>@<domain_id>", "alex@morgan"
    "Domain ID", "domain where the account was created", "RFC1035 [#f1]_, RFC1123 [#f2]_ ", "morgan"
    "Quorum", "number of signatories needed to sign the transaction to make it valid", "0 < quorum ≤ 128", "5"
    "JSON data", "key-value account information", "JSON", "{ genesis: {name: alex} }"

Get Signatories
^^^^^^^^^^^^^^^

Purpose
-------

Purpose of get signatories query is to get signatories, which act as an identity of the account.

Request Schema
--------------

.. code-block:: proto

    message GetSignatories {
        string account_id = 1;
    }

Request Structure
-----------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "account id to request signatories", "<account_name>@<domain_id>", "alex@morgan"

Response Schema
---------------

.. code-block:: proto

    message SignatoriesResponse {
        repeated bytes keys = 1;
    }

Response Structure
------------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Keys", "an array of public keys", "`ed25519 <https://ed25519.cr.yp.to>`_", "292a8714694095edce6be799398ed5d6244cd7be37eb813106b217d850d261f2"

Get Transactions
^^^^^^^^^^^^^^^^

Purpose
-------

GetTransactions is used for retrieving information about transactions, based on their hashes.

Request Schema
--------------

.. code-block:: proto

    message GetTransactions {
        repeated bytes tx_hashes = 1;
    }

Request Structure
-----------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Transactions hashes", "an array of hashes", "array with 32 byte hashes", "{hash1, hash2…}"

Response Schema
---------------

.. code-block:: proto

    message TransactionsResponse {
        repeated Transaction transactions = 1;
    }

Response Structure
------------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Transactions", "an array of transactions", "Committed transactions", "{tx1, tx2…}"

Get Pending Transactions
^^^^^^^^^^^^^^^^^^^^^^^^

Purpose
-------

GetPendingTransactions is used for retrieving a list of pending (not fully signed) `multisignature transactions <../core_concepts/glossary.html#multisignature-transactions>`_
or `batches of transactions <../core_concepts/glossary.html#batch-of-transactions>`__ issued by account of query creator.

Request Schema
--------------

.. code-block:: proto

    message GetPendingTransactions {
    }

Response Schema
---------------

.. code-block:: proto

    message TransactionsResponse {
        repeated Transaction transactions = 1;
    }

Response Structure
------------------

The response contains a list of `pending transactions <../core_concepts/glossary.html#pending-transactions>`_.

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

        "Transactions", "an array of pending transactions", "Pending transactions", "{tx1, tx2…}"


Get Account Transactions
^^^^^^^^^^^^^^^^^^^^^^^^

Purpose
-------

In a case when a list of transactions per account is needed, `GetAccountTransactions` query can be formed.

Request Schema
--------------

.. code-block:: proto

    message GetAccountTransactions {
        string account_id = 1;
    }

Request Structure
-----------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "account id to request transactions from", "<account_name>@<domain_id>", "makoto@soramitsu"

Response Schema
---------------

.. code-block:: proto

    message TransactionsResponse {
        repeated Transaction transactions = 1;
    }

Response Structure
------------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Transactions", "an array of transactions for given account", "Committed transactions", "{tx1, tx2…}"

Get Account Asset Transactions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Purpose
-------

`GetAccountAssetTransactions` query returns all transactions associated with given account and asset.

Request Schema
--------------

.. code-block:: proto

    message GetAccountAssetTransactions {
        string account_id = 1;
        string asset_id = 2;
    }

Request Structure
-----------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "account id to request transactions from", "<account_name>@<domain_id>", "makoto@soramitsu"
    "Asset ID", "asset id in order to filter transactions containing this asset", "<asset_name>#<domain_id>", "jpy#japan"

Response Schema
---------------

.. code-block:: proto

    message TransactionsResponse {
        repeated Transaction transactions = 1;
    }

Response Structure
------------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Transactions", "an array of transactions for given account and asset", "Committed transactions", "{tx1, tx2…}"

Get Account Assets
^^^^^^^^^^^^^^^^^^

Purpose
-------

To get the state of all assets in an account (a balance), `GetAccountAssets` query can be used.

Request Schema
--------------

.. code-block:: proto

    message GetAccountAssets {
        string account_id = 1;
    }

Request Structure
-----------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Account ID", "account id to request balance from", "<account_name>@<domain_id>", "makoto@soramitsu"

Response Schema
---------------
.. code-block:: proto

    message AccountAssetResponse {
        repeated AccountAsset acct_assets = 1;
    }

    message AccountAsset {
        string asset_id = 1;
        string account_id = 2;
        Amount balance = 3;
    }

Response Structure
------------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Asset ID", "identifier of asset used for checking the balance", "<asset_name>#<domain_id>", "jpy#japan"
    "Account ID", "account which has this balance", "<account_name>@<domain_id>", "makoto@soramitsu"
    "Balance", "balance of the asset", "Not less than 0", "200.20"

Get Account Detail
^^^^^^^^^^^^^^^^^^

Purpose
-------

To get details of the account, `GetAccountDetail` query can be used. Account details are key-value pairs, splitted into writers categories. Writers are accounts, which added the corresponding account detail. Example of such structure is:

.. code-block:: json

    {
        "account@a_domain": {
            "age": 18,
            "hobbies": "crypto"
        },
        "account@b_domain": {
            "age": 20,
            "sports": "basketball"
        }
    }

Here, one can see four account details - "age", "hobbies" and "sports" - added by two writers - "account@a_domain" and "account@b_domain". All of these details, obviously, are about the same account.

Request Schema
--------------

.. code-block:: proto

    message GetAccountDetail {
      oneof opt_account_id {
        string account_id = 1;
      }
      oneof opt_key {
        string key = 2;
      }
      oneof opt_writer {
        string writer = 3;
      }
    }

.. note::
    Pay attention, that all fields are optional. Reasons will be described later.

Request Structure
-----------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

        "Account ID", "account id to get details from", "<account_name>@<domain_id>", "account@domain"
        "Key", "key, under which to get details", "string", "age"
        "Writer", "account id of writer", "<account_name>@<domain_id>", "account@domain"

Response Schema
---------------

.. code-block:: proto

    message AccountDetailResponse {
      string detail = 1;
    }

Response Structure
------------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

        "Detail", "key-value pairs with account details", "JSON", "see below"

Usage Examples
--------------

Let's again consider the example of details from the beginning and see, how different variants of `GetAccountDetail` queries will change the resulting response.

.. code-block:: json

    {
        "account@a_domain": {
            "age": 18,
            "hobbies": "crypto"
        },
        "account@b_domain": {
            "age": 20,
            "sports": "basketball"
        }
    }

**account_id is not set**

If account_id is not set - other fields can be empty or not - it will automatically be substituted with query creator's account, which will lead to one of the next cases.

**only account_id is set**

In this case, all details about that account are going to be returned, leading to the following response:

.. code-block:: json

    {
        "account@a_domain": {
            "age": 18,
            "hobbies": "crypto"
        },
        "account@b_domain": {
            "age": 20,
            "sports": "basketball"
        }
    }

**account_id and key are set**

Here, details added by all writers under the key are going to be returned. For example, if we asked for the key "age", that's the response we would get:

.. code-block:: json

    {
        "account@a_domain": {
            "age": 18
        },
        "account@b_domain": {
            "age": 20
        }
    }

**account_id and writer are set**

Now, the response will contain all details about this account, added by one specific writer. For example, if we asked for writer "account@b_domain", we would get:

.. code-block:: json

    {
        "account@b_domain": {
            "age": 20,
            "sports": "basketball"
        }
    }

**account_id, key and writer are set**

Lastly, if all three field are set, result will contain details, added the specific writer and under the specific key, for example, if we asked for key "age" and writer "account@a_domain", we would get:

.. code-block:: json

    {
        "account@a_domain": {
            "age": 18
        }
    }

Get Asset Info
^^^^^^^^^^^^^^

Purpose
-------

In order to know precision for given asset, and other related info in the future, such as a description of the asset, etc. user can send `GetAssetInfo` query.

Request Schema
--------------

.. code-block:: proto

    message GetAssetInfo {
        string asset_id = 1;
    }

Request Structure
-----------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Asset ID", "asset id to know related information", "<asset_name>#<domain_id>", "jpy#japan"


Response Schema
---------------

.. code-block:: proto

    message Asset {
        string asset_id = 1;
        string domain_id = 2;
        uint32 precision = 3;
    }

.. note::
    Please note that due to a known issue you would not get any exception if you pass invalid precision value.
    Valid range is: 0 <= precision <= 255

Response Structure
^^^^^^^^^^^^^^^^^^

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Asset ID", "identifier of asset used for checking the balance", "<asset_name>#<domain_id>", "jpy"
    "Domain ID", "domain related to this asset", "RFC1035 [#f1]_, RFC1123 [#f2]_", "japan"
    "Precision", "number of digits after comma", "0 <= precision <= 255", "2"

Get Roles
^^^^^^^^^

Purpose
-------

To get existing roles in the system, a user can send `GetRoles` query to Iroha network.

Request Schema
--------------

.. code-block:: proto

    message GetRoles {
    }

Response Schema
---------------

.. code-block:: proto

    message RolesResponse {
        repeated string roles = 1;
    }

Response Structure
------------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Roles", "array of created roles in the network", "set of roles in the system", "{MoneyCreator, User, Admin, …}"

Get Role Permissions
^^^^^^^^^^^^^^^^^^^^

Purpose
-------

To get available permissions per role in the system, a user can send `GetRolePermissions` query to Iroha network.

Request Schema
--------------

.. code-block:: proto

    message GetRolePermissions {
        string role_id = 1;
    }

Request Structure
-----------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Role ID", "role to get permissions for", "existing role in the system", "MoneyCreator"

Response Schema
---------------

.. code-block:: proto

    message RolePermissionsResponse {
        repeated string permissions = 1;
    }

Response Structure
------------------

.. csv-table::
    :header: "Field", "Description", "Constraint", "Example"
    :widths: 15, 30, 20, 15

    "Permissions", "array of permissions related to the role", "string of permissions related to the role", "{can_add_asset_qty, …}"

.. [#f1] https://www.ietf.org/rfc/rfc1035.txt
.. [#f2] https://www.ietf.org/rfc/rfc1123.txt
