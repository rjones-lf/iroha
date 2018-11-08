Python library
==============

How to install iroha python library
-----------------------------------

To install latest release: ``bash pip install iroha`` To install
developer version:
``bash pip install -i https://testpypi.python.org/pypi iroha``

How to build and install iroha locally
--------------------------------------

``bash python setup.py install``

How to build and publish iroha
------------------------------

Creating MacOS wheel and publishing it:
``bash python setup.py bdist_wheel twine upload --repository-url https://pypi.org/legacy/ dist/*``
Creating Linux source distribution and publishing it:
``bash python setup.py sdist twine upload --repository-url https://pypi.org/legacy/ dist/*``


Python client library example
=============================

Prerequisites
*************

In order to execute script demonstrating execution of python client library you need to have python 2.7 installed. After that please follow next steps:

* To compile ``grpc proto`` files it is needed to have ``grpc-io`` installed:

``pip2 install grpcio-tools``

* Change to */example/python* Directory.

``cd /example/python``

* Run prepare.sh script to build iroha python library and compile proto files:

``./prepare.sh``

* Make sure you have running iroha on your machine. You can follow this guide to launch iroha daemon. Please run iroha from iroha/example folder, since python script uses keys from there.

Launch example
**************

Script **tx-example.py** does the following:

* Assemble transaction from several commands using **tx_builder**
* Sign it using keys from iroha/example folder 
* Send it to iroha
* Wait 5 secs and check transaction's status using its hash
* Assemble query using **query_builder**
* Send query to iroha
* Read query response

Launch it:
**********

``python2 tx-example``

.. WARNING:: Docs are constantly updated and this section is going to be improved.
