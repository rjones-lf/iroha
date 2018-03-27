Client Libraries
================

C++ Library
-----------

Where to Get
^^^^^^^^^^^^

How to Use/Import
^^^^^^^^^^^^^^^^^

Example Code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^

Java Library
------------

Where to Get
^^^^^^^^^^^^

How to Use/Import
^^^^^^^^^^^^^^^^^

Example Code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^

Android Library
---------------

The library, in essence, is a set of Java interfaces and binary libraries compiled for different architectures. Supported architectures are *arm, x86, x86_64*.

Where to Get
^^^^^^^^^^^^

There are two ways to get Iroha library for Android:

#. Grab via Gradle (see details in the section `Importing the Library from jcenter`_)

   .. code-block:: groovy

       implementation 'jp.co.soramitsu.iroha.android:iroha-android-bindings:1.0'

#. Compile the library on your own.

Both options are described in the following sections.

Manual Build
^^^^^^^^^^^^

The guide was tested on systems running Ubuntu 16.04 and macOS.

Prerequisites
"""""""""""""

Android NDK
    Please `download <https://developer.android.com/ndk/downloads/index.html>`__ and unpack NDK to any suitable folder.

automake
    .. code-block:: shell

        sudo apt install automake
        automake --version
        # automake (GNU automake) 1.15

bison
    .. code-block:: shell

        sudo apt install bison
        bison --version
        # bison (GNU Bison) 3.0.4

cmake
    Minimum required version is 3.8, but we recommend to install the latest available version (3.10.3 at the moment).
    
    Since Ubuntu repositories contain unsuitable version of cmake, you need to install the new one manually.
    Here is how to build and install cmake from sources.

    .. code-block:: shell

        wget https://cmake.org/files/v3.10/cmake-3.10.3.tar.gz
        tar -xvzf cmake-3.10.3.tar.gz
        cd cmake-3.10.3/
        ./configure
        make
        sudo make install
        cmake --version
        # cmake version 3.10.3


Building the Library
""""""""""""""""""""

All you need now is to download `build script <https://github.com/hyperledger/iroha/blob/develop/example/Android/android-build.sh>`__
``android-build.sh`` to any empty folder and launch it there. 

Launch parameters are listed in the table below.

+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| Position | Required | Parameter Name        | Description                                 | Possible Values                     |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| 1        | Yes      | **Platform Name**     | Name of the target platform for binary      | ``arm64-v8a``, ``armeabi-v7a``,     |
|          |          |                       | part of the library.                        | ``armeabi``, ``x86``, ``x86_64``    |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| 2        | Yes      | **Android API Level** | API level supported by your NDK.            | ``27`` for android-ndk-r16b         |
|          |          |                       | See the link under the table for details.   |                                     |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| 3        | Yes      | **Android NDK Path**  | Full path to unpacked NDK. Please           | ``/home/user/lib/android-ndk-r16b`` |
|          |          |                       | ensure that path does not contain spaces.   |                                     |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| 4        | Yes      | **Java Package Name** | Package name that will be used for Java     | ``jp.co.soramitsu.iroha.android``   |
|          |          |                       | interfaces generation. Note that the binary |                                     |
|          |          |                       | also depends on chosen package name.        |                                     |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+
| 5        | No       | **Build Type**        | Defines build mode of binary part           | ``Debug`` or ``Release``            |
|          |          |                       | of the library. ``Release`` is the default  |                                     |
|          |          |                       | option.                                     |                                     |
+----------+----------+-----------------------+---------------------------------------------+-------------------------------------+

`Android API levels <https://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels>`__

Please use the same root part of Java package name for library build as you use for your Android project.
For example, your project is located in a package called ``com.mycompany.androidapp``, so please consider to build the library in a
package, which name starts with ``com.mycompany.androidapp`` (e.g. ``com.mycompany.androidapp.iroha``).

A couple of launch commands examples:

.. code-block:: shell

    # build Java bindings and binary library for arm64-v8a in Release mode
    ./android-build.sh arm64-v8a 27 /home/user/lib/android-ndk-r16b com.mycompany.iroha

    # build Java bindings and binary library for x86 in Debug mode
    ./android-build.sh x86 27 /home/user/lib/android-ndk-r16b com.mycompany.iroha Debug

Build artefacts will be collected in ``lib`` directory near the script ``android-build.sh``.
There will be two files - an archive ``bindings.zip`` and ``libirohajava.so``.


How to Use/Import
^^^^^^^^^^^^^^^^^

Importing the Library from jcenter
""""""""""""""""""""""""""""""""""

The easiest way to use Irohalib for Android is to import the library dependency from `jcenter <https://bintray.com/bulatmukhutdinov/maven/iroha-android-bindings>`__.

All you need to do is a simple set of four steps:

1. Add to your ``build.gradle`` file the following line:

   .. code-block:: groovy

       implementation 'jp.co.soramitsu.iroha.android:iroha-android-bindings:1.0'

2. Copy the latest version of ``*.proto`` files from ``develop`` branch of Iroha `repository <https://github.com/hyperledger/iroha/tree/develop/schema>`__ into 
   ``app/src/main/proto/`` folder inside your project in Android Studio.

   The resulting directory structure should look like as follows:

   .. code-block:: shell

        app
        └── src
            └── main
                └── proto
                    ├── google
                    │   └── protobuf
                    │       └── empty.proto
                    ├── block.proto
                    ├── commands.proto
                    ├── endpoint.proto
                    ├── loader.proto
                    ├── ordering.proto
                    ├── primitive.proto
                    ├── proposal.proto
                    ├── queries.proto
                    ├── responses.proto
                    └── yac.proto


3. Create additional directories ``app/src/main/proto/google/protobuf/`` and place there a file called ``empty.proto`` with the following contents:

   .. code-block:: proto

       syntax = "proto3";

       package google.protobuf;

       option java_package = "com.google.protobuf";
       option java_outer_classname = "EmptyProto";
       option java_multiple_files = true;

       message Empty {
       }

4. Add ``protobuf`` and ``grpc`` dependecies and protobuf configuration block into your ``buld.gradle`` file.

   .. code-block:: groovy

        apply plugin: 'com.google.protobuf' 

        dependencies {
            ...

            implementation 'com.google.protobuf:protobuf-lite:3.0.1'
            implementation 'io.grpc:grpc-core:1.8.0'
            implementation 'io.grpc:grpc-stub:1.8.0'
            implementation 'io.grpc:grpc-okhttp:1.8.0'
            implementation('io.grpc:grpc-protobuf-lite:1.8.0') {
            // Otherwise Android compile will complain "Multiple dex files define ..."
            exclude module: "protobuf-lite"
        }

        protobuf {
            protoc {
                artifact = 'com.google.protobuf:protoc:3.5.1-1'
            }
            plugins {
                javalite {
                    artifact = "com.google.protobuf:protoc-gen-javalite:3.0.0"
                }
                grpc {
                    artifact = 'io.grpc:protoc-gen-grpc-java:1.10.0'
                }
            }
            generateProtoTasks {
                all().each { task ->
                    task.plugins {
                        javalite {}
                        grpc {
                            // Options added to --grpc_out
                            option 'lite'
                            option 'generate_equals=true'
                        }
                    }
                }
            }
        }

How to Use Manually Built Library
"""""""""""""""""""""""""""""""""

1. Create directory structure inside your Android project according to the package name of build library.
   Put there all the ``.java`` files from ``bindings.zip`` archive. 
   For example, the path could be ``app/src/main/java/com/mycompany/iroha`` if you built the library with 
   ``com.mycompany.iroha`` package name.

2. Create directory ``app/src/main/jniLibs/<platform>`` where ``<platform>`` is the name of target platform
   (e.g. ``arm64-v8a``). Put there ``libirohajava.so``. Repeat this step for all required platforms
   (in this case you need to build the library for each platform).

3. Repeat steps 2-4 from the previous section `Importing the Library from jcenter`_.


Example Code
^^^^^^^^^^^^

Explore ``bindings`` branch of `iroha-android <https://github.com/hyperledger/iroha-android/tree/bindings>`__ repository to get source code and view sample application.


Objective-C Library
-------------------

Where to Get
^^^^^^^^^^^^

How to Use/Import
^^^^^^^^^^^^^^^^^

Example Code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^

Swift Library
-------------

Where to Get
^^^^^^^^^^^^

How to Use/Import
^^^^^^^^^^^^^^^^^

Example Code
^^^^^^^^^^^^

Troubleshoting
^^^^^^^^^^^^^^

Python Library
--------------

Where to Get
^^^^^^^^^^^^

How to Use/Import
^^^^^^^^^^^^^^^^^

Example Code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^

NodeJS Library
--------------

Where to Get
^^^^^^^^^^^^

How to Use/Import
^^^^^^^^^^^^^^^^^

Example Code
^^^^^^^^^^^^

Troubleshooting
^^^^^^^^^^^^^^^