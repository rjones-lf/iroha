Node.js Library
---------------
.. warning:: Please note that Node.js library is under heavy testing now and
  problems `might` occur. Don't hesitate to report them to us.

There are two main ways of obtaining the Node.js library. If you are a happy
macOS or Linux user, you can install it `through NPM <#installing-through-npm>`_.
If your system is not yet supported or you want to try the latest version, you
can `build this library manually <#building-manually>`_.

Prerequisites
^^^^^^^^^^^^^

- Node.js (>=7) (you can try using lower versions though).

Installing Through NPM
^^^^^^^^^^^^^^^^^^^^^^
If you are a happy macOS or Linux user, you can install the library from `NPM
repository <https://www.npmjs.com/package/iroha-lib>`_ using NPM

.. code-block:: shell

  npm install iroha-lib

Now you can import it in your project

.. code-block:: javascript

  const iroha = require('iroha-lib')

Building Manually
^^^^^^^^^^^^^^^^^
You need this section if you want to build iroha-lib manually for example if
your architecture/OS is not supported yet.

Prerequisites
"""""""""""""
1. CMake (>=3.8.2)
2. Protobuf (>=3.5.1)
3. Boost (>=1.65.1)

macOS users can install dependencies with following commands:

.. code-block:: shell

  brew install node cmake # Common dependencies
  brew install autoconf automake ccache # SWIG dependencies
  brew install protobuf boost # Iroha dependencies

.. warning:: If you have SWIG already installed, you **MUST** install patched
  3.0.12 version instead using
  `this patch <https://patch-diff.githubusercontent.com/raw/swig/swig/pull/968.patch>`_.
  The current version of SWIG doesn't support Node.js versions higher than 6.
  Also you can just delete the global installed SWIG and iroha will pull and
  build it automatically.

Build Process
"""""""""""""
Clone Iroha repository

.. code-block:: shell

  git clone -b develop --depth=1 https://github.com/hyperledger/iroha

Go to the NPM package directory and start the build process

.. code-block:: shell

  cd iroha/shared_model/packages/javascript
  npm install

That's all. You can use the library now.

Example code
^^^^^^^^^^^^
Explore ``example/node/index.js`` file to get an idea of how to
work with a library.

Prerequisites
"""""""""""""
To run this example, you need an Iroha node up and running. Please check out
:ref:`getting-started` if you want to learn how to start it.

Running the Example
"""""""""""""""""""
To start the example, you need to install all its dependencies
(basically ``iroha-lib``)

.. code-block:: shell

  npm install

.. note:: if you have built the library manually, please change import paths
  to ``path-to-iroha/shared_model/packages/javascript`` in index.js

Now, to run this example please write:

.. code-block:: shell

  node index.js
