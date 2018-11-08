NodeJS library
==============

NodeJS client library example
-----------------------------

Prerequisites
*************

Make sure you have running iroha on your machine. You can follow this guide to launch iroha daemon. Please use keys for iroha from iroha/example folder, since this example uses keys from there.

If you are a lucky owner of a processor with the x64 architecture, you can install iroha-lib from the NPM repository with a simple command:

``npm install iroha-lib``

In other cases, you need to download the complete Iroha repository (in which you are now), go to the ``shared_model/packages/javascript`` folder and build the package on your system manually using the instructions from README.md. In such case, you need to change the import paths in this example to ``shared_model/packages/javascript``.

Launch example
**************

Script **index.js** does the following:

* Assemble transaction from several commands using tx builder
* Sign it using keys from iroha/example folder
* Send it to iroha
* Wait 5 secs and check transaction's status using its hash
* Assemble query using query builder
* Send query to iroha
* Read query response

Launch it:
**********

node index.js

.. WARNING:: Docs are constantly updated and this section is going to be improved.




