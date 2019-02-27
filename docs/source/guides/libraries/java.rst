Java Library
------------

The source code of the library, which can be used with Java and Kotlin projects can be found here: https://github.com/hyperledger/iroha-java/

Please, read corresponding README.md file in aim to get more actual information

Prerequisites
^^^^^^^^^^^^^

- Java 8 / Kotlin
- Gradle


How to Use
^^^^^^^^^^

To use Iroha in your Java/Kotlin project you should declare a dependency in the ``build.gradle`` file:

.. code-block:: groovy

  compile "com.github.hyperledger.iroha-java:client:$irohaJavaVersion"

The version of the library release should correspond to the version of the Iroha. 
You can find information about this correspondence at the GitHub page of the library: https://github.com/hyperledger/iroha-java/releases
For example, if you are using Iroha version `1.0 RC3`. then you should use version `2.0.0` of Iroha Java library:

.. code-block:: groovy

  ext {
    irohaJavaVersion = '2.0.0'
  }


Example code
^^^^^^^^^^^^
Explore ``client/src/test/java/jp/co/soramitsu/iroha/java`` file to get an idea of how to
work with a library.

