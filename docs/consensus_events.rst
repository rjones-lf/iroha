.. _devGuide:

.. highlight:: rst

==================================
Consensus events
==================================

This site covers Iroha's usage & API documentation. For basic info on what
Iroha is, please see `the main project website <http://iroha.tech>`_.


Transactions
------------

Though binary support is planned in the future, all transactions in Iroha are currently serialized as JSON.

Message transactions
********************

.. code-block:: json

   {
       "command" : "message",
       "message" : "[arbitrary string]",
       "publicKey" : "[base64 public key]",
       "signature" : "[base64 signature]",
       "version" : "[optional]"
   }

Membership
----------

Trust system (Hijiri)
---------------------

「聖の字をヒジリと読むのは、非を知るという意味。」山本常朝

