Account
=======

An abstraction that represents users of Iroha.
Each account is related to one of existing `domains <#domain>`__.
Account can be granted with one or more roles.

Only `grantable permissions <#grantable-permission>`__ can be granted to an account directly.

Ametsuchi
=========

Internal Iroha storage.
There is no legal way for the `client <#client>`__ to directly interact with Ametsuchi.

Asset
=====

Any unit that could be a subject of accounting.
Each asset is related to one of existing `domains <#domain>`__.
For example, asset can represent any kind of countable units - money, gold, etc.

Block Creator
=============

Components that transforms a set of transactions that have been passed `stateless <#stateless-validation>`__ and `stateful <#stateful-validation>`__ validation into a block for further propogation to `consensus <#consensus>`__.

Client
======

Any application that uses Iroha is treated as a client.

Command
=======

Command is a request to Iroha that **does** change the `state <#world-state-view>`__.
All modifications are performed via commands.
For example, in order to create a new `role <#role>`__ in Iroha you have to issue `Create role <../api/commands.html#create-role>`__ command.

Consensus
=========

Internal component that preserves consistent state among the `peers <#peer>`__ within an Iroha cluster.
Iroha uses own consensus algorithm called Yet Another Consensus (aka YAC).
YAC able to apply only the latest block to the chain.
If there are missing blocks, they will be downloaded from another peers via `Synchronizer <#synchronizer>`__.

Committed blocks are stored in `Ametsuchi <#ametsuchi>`__.

Domain
======

A named abstraction for grouping `accounts <#account>`__ and `assets <#asset>`__.

Ordering Gate
=============

Internal Iroha component that passess `transactions <#transaction>`__ from `Peer Communication Service <#peer-communication-service>`__ to `Ordering Service <#ordering-service>`__.
Ordering Gate eventually recieves `proposals <#proposal>`__ from Ordering Service and sends them to `Simulator <#simulator>`__ for `stateful validation <#stateful-validation>`__.

Ordering Service
================

Internal Iroha component that combines several `transactions <#transaction>`__ that have been passed `stateless validation <#stateless-validation>`__ into a `proposal <#proposal>`__.
Proposal creation could be triggerd by one of the following events:

1. Time limit dedicated for transactions collection has expired.

2. Ordering service has received maximum amount of transactions allowed for a single proposal.

Both parameters (timeout and maximum size of proposal) are configurable.

A common precondition for both triggers is that at least one transaction should reach ordering service. Otherwise, no proposal will be formed.

Peer
====

A node that is a part of cluster running Iroha.

Peer Communication Service
==========================

Internal component of Iroha - an intermediary that transmits `transaction <#transaction>`__ from `Torii <#torii>`__ to `Ordering Gate <#ordering-gate>`__.
The main goal of PCS is to hide the complexity of interaction with consensus implementation from `Torii <#torii>`__.

Permission
==========

A named rule that prescribes allowance to do a particular thing.
Permission **cannot** be granted to an `account <#account>`__ directly.
Permission can be granted to an account only within roles.

Grantable Permission
--------------------

Only grantable permission be granted to an `account <#account>`__ directly.
An account that holds grantable permission is allowed to perform some particular action on behalf of another account.

Proposal
========

A set of `transactions <#transaction>`__ that have been passed only `stateless validation <#stateless-validation>`__.

Verified Proposal
-----------------

A set of transactions that have been passed `stateless <#stateless-validation>`__ and `stateful <#stateful-validation>`__ validation.

Role
====

A named abstraction that holds a set of `permissions <#permission>`__.

Simulator
=========

See `Verified Proposal Creator <#verified-proposal-creator>`__.

Query
=====

A request to Iroha that does **not** change the `state <#world-state-view>`__.

Synchronizer
============

Is a part of `consensus <#consensus>`__.
Adds missing blocks into `peers <#peer>`__' chains (downloads them from other peers).

Torii
=====

⛩.
Entry point for `clients <#client>`__.
Uses gRPC as a transport.

Transaction
===========

An ordered set of `commands <#command>`__, which can be applied to the chain only in atomic way.
Any non valid command within a transaction lead to drop of the whole transaction.


Validator
=========

There are two kinds of validation - stateless and stateful.

Stateless Validation
--------------------

Performed in `Torii <#torii>`__.
Represents all checks that do not depend on `World State View <#world-state-view>`__.

Stateful Validation
-------------------

Performed in `Verified Proposal Creator <#verified-proposal-creator>`__.
Validates against `World State View <#world-state-view>`__.

Verified Proposal Creator
=========================

Internal Iroha component that performs `stateful validation <#stateful-validation>`_ of `transactions <#transaction>`__ contained in received `proposal <#proposal>`__.
On the basis of transactions that have been passed stateful validation **verified proposal** will be created and passed to `Block Creator <#block-creator>`__.
All the transactions that has not passed stateful validation will be dropped and not included into verified proposal.

World State View
================

WSV reflects the current state of the system.
For example, WSV holds information about amount of `assets <#asset>`__ assigned to `accounts <#account>`__ at the moment, but does not contains any info about `transactions <#transaction>`__ history.
