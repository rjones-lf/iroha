Java Library
------------

Prerequisites
^^^^^^^^^^^^^

- Java 6
- Gradle

Build Process
^^^^^^^^^^^^^

Clone Iroha repository

.. code-block:: shell

  git clone https://github.com/hyperledger/iroha.git --depth=1
  cd iroha

.. note:: For the latest version checkout a ``develop`` branch by adding
  ``-b develop`` parameter.

Now we need to build our Java native library. Run
``example/java/build_library.sh``, which will take care of it.

.. code-block:: shell

  cd example/java
  ./build_library.sh

.. note:: ``build_library.sh`` script creates a ``dist`` folder with
  files, needed in :ref:`java-how-to-use` section.

.. _java-how-to-use:

How to Use
^^^^^^^^^^

There are two ways of adding the Java library to your project:

1. Import Java bindings through Maven Central
2. Compile Java bindings manually

Both options are described in the following sections.

Import Java Bindings from Maven Central
"""""""""""""""""""""""""""""""""""""""

First of all, you need to copy ``example/java/dist/libirohajava.jnilib`` to
the root folder of your project.

.. code-block:: shell

  cp dist/libirohajava.jnilib /path_to_your_project

If you use **Gradle**, add the following line to your ``build.gradle`` file:

.. code-block:: groovy
  :caption: build.gradle

      compile group: 'jp.co.soramitsu', name: 'iroha', version: ‘0.0.7’

    If you use **Maven**, add this to your ``pom.xml``:

.. code-block:: xml
  :caption: pom.xml

      <!-- https://mvnrepository.com/artifact/jp.co.soramitsu/iroha -->
  <dependency>
      <groupId>jp.co.soramitsu</groupId>
      <artifactId>iroha</artifactId>
      <version>0.0.7</version>
  </dependency>

.. note:: Set the latest version number from our
  `Maven repository <https://mvnrepository.com/artifact/jp.co.soramitsu/iroha>`_

Compiling Java Bindings Manually
""""""""""""""""""""""""""""""""

Java bindings were compiled with ``example/java/build_library.sh`` in
`Build Process` section. You need to copy ``example/java/dist/libirohajava.jnilib``
to the root folder of your project:

.. code-block:: shell

  cp dist/libirohajava.jnilib /path_to_your_project

If you use **Gradle**, you need to copy ``example/java/dist/iroha-bindings-example.jar`` to the
``libs`` folder of your project

.. code-block:: shell

  cp dist/iroha-bindings-example.jar /path_to_your_project/libs

Then please add the following to your ``build.gradle`` file:

.. code-block:: groovy
  :caption: build.gradle

      dependencies {
        compile fileTree(dir: 'libs', include: ['*.jar'])
      }

    If you use **Maven**, you need to copy ``example/java/dist/iroha_lib.jar`` to the
  ``src/main/resources/`` folder of your project

.. code-block:: shell

  cp dist/iroha_lib.jar /path_to_your_project/src/main/resources

After it please add this to your ``pom.xml``:

.. code-block:: xml
  :caption: pom.xml

      <dependency>
          <groupId>jp.co.soramitsu</groupId>
          <artifactId>iroha</artifactId>
          <version>0.0.7</version>
          <systemPath>${project.basedir}/src/main/resources/iroha_lib.jar</systemPath>
      </dependency>

Example code
^^^^^^^^^^^^
Explore ``example/java/TransactionExample.java`` file to get an idea of how to
work with a library.

Prerequisites
"""""""""""""
To run this example, you need an Iroha node up and running. Please check out
:ref:`getting-started` if you want to learn how to start it.

Running the Example
"""""""""""""""""""
To start the example, you need to build the example:

.. code-block:: shell

  ./prepare.sh
  gradle build

Now, to run this example please write:

.. code-block:: shell

  gradle run
