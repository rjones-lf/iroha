iOS Swift Library
-----------------

Objectives
^^^^^^^^^^

In this guide you will learn:

-	How to build client library for iOS
-	How to configure test application
-	How to interact with Iroha blockchain from the mobile device

Video Guide
^^^^^^^^^^^

For more details please visit the video below which fully describes all the steps.

.. raw:: html

    <iframe width="560" height="315" style="margin-bottom: 20px" src="https://www.youtube.com/embed/sjuK3I1I080" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

Prerequisites
^^^^^^^^^^^^^

Before starting you need to install the following software on your mac:

-	XCode
-	Carthage
-	Git
-	Cmake
-	Postgresql

This tutorial was tested with the following environment:

-	MacOS Sierra 10.12.6
-	Xcode 9.2
-	carthage 0.29.0
-	cmake 3.11.4
-	iPhone 7 iOS 11.2 Simulator
 
Hyperledger Iroha iOS library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Iroha has the following features:

1.	Creation and management of custom complex assets, such as currencies or indivisible rights, serial numbers, patents, etc.
2.	Management of user accounts
3.	Taxonomy of accounts based on domains — or sub-ledgers in the system
4.	The system of rights and verification of user permissions for the execution of transactions and queries in the system
5.	Validation of business rules for transactions and queries in the system

Among the non-functional requirements can be noted a high degree of network fault tolerance (Byzantine Fault Tolerant).
Iroha iOS library gives the ability to provide key generation and signing logic for queries and transactions passed to Iroha blockchain.
Let's start with the detailed instructions how to install Iroha on the local machine.


 
Instruction
^^^^^^^^^^^

1.	Open the terminal and go to the folder where you want to install all artifacts:

  .. code-block:: bash

      cd path/to/your/folder/for/example/iroha-ios/project/

2.	Clone the repository for the iOS client:

  .. code-block:: bash

      git clone https://github.com/hyperledger/iroha-ios.git

3.	Go to the Iroha-ios folder:

  .. code-block:: bash

      cd iroha-ios/

4.	Update dependencies:

  .. code-block:: bash

      carthage update --platform iOS

5.	Go to sample project directory:

  .. code-block:: bash

      cd SwiftyIrohaExample

6.	Update dependencies for the sample:

  .. code-block:: bash

      carthage update --platform iOS

7.	Go to GRPC library source's location:

  .. code-block:: bash

      cd grpc-swift/

8.	Remove old library sources:

.. note:: Make sure you are located in ``grpc-swift/`` subfolder

  .. code-block:: bash

      # removes all files from the current directory
      rm -rf ./*
      # removes all hidden files too (so clean build can be done)
      rm -rf ./.*
 
9.	Download release version of GRCP from git to the current directory:

  .. code-block:: bash

      git clone --branch 0.3.3 https://github.com/grpc/grpc-swift.git .

10.	Build library:

  .. code-block:: bash

      make

11.	Go to the root of your playground folder (from the first step - path/to/your/folder/for/example/iroha-ios/project/):

  .. code-block:: bash

      cd ../../..

.. note:: Make sure now you are located in ``path/to/your/folder/for/example/iroha-ios/project/`` folder

12.	This step downloads script for client library which is needed to build client library. Clone it from the repository:

  .. code-block:: bash

      curl https://raw.githubusercontent.com/hyperledger/iroha/master/shared_model/packages/ios/ios-build.sh > ios-build.sh

13.	Optional step. If you have issues with cloning during ios-build.sh execution do the following command before the script invocation:

  .. code-block:: bash

      sed -i '' 's|git://github.com/hyperledger/iroha-ed25519|https://github.com/hyperledger/iroha-ed25519.git|g' ios-build.sh

14.	Make downloaded script executable:

  .. code-block:: bash

      chmod +x ios-build.sh

15.	Finally, build the client iOS library with proper options - platform: OS | SIMULATOR | SIMULATOR64; build: Debug | Release :

  .. code-block:: bash

      ./ios-build.sh SIMULATOR64 Debug

16.	The generated artifacts should be copied to the proper location (let's create it first):

  .. code-block:: bash

      # this command shows location for simulator artifacts
      # use this command for device instead:
      # mkdir -p iroha-ios/libs/iOS/
      mkdir -p iroha-ios/libs/Simulator/
 
17.	Copy generated binaries:

  .. code-block:: bash

      # this command shows location for simulator artifacts
      # use this command for device instead:
      # cp lib/* iroha-ios/libs/iOS/
      cp lib/* iroha-ios/libs/Simulator/

18.	Do not forget to copy generated headers:

  .. code-block:: bash

      cp -a include/. iroha-ios/headers/

19.	Now it's time to manually config Xcode project for the sample application. Open SwiftyIroha.xcodeproj:

.. image:: https://github.com/hyperledger/iroha/raw/develop/docs/image_assets/iroha_swift_guide/iroha_swift_guide_001.png
 
20.	Select SwiftyIrohaExample.xcodeproj general tab and link SwiftProtobuf framework from iroha-ios/SwiftProtobuf.framework location

.. image:: https://github.com/hyperledger/iroha/raw/develop/docs/image_assets/iroha_swift_guide/iroha_swift_guide_002.png

21.	Select SwiftGRPC.xcodeproj project and remove zlib-example target from it:

.. image:: https://github.com/hyperledger/iroha/raw/develop/docs/image_assets/iroha_swift_guide/iroha_swift_guide_003.png
 
22.	Go to Proto group and remove it (In future this step will be removed, but for now it's needed for sample app to be built):

.. image:: https://github.com/hyperledger/iroha/raw/develop/docs/image_assets/iroha_swift_guide/iroha_swift_guide_004.png

23.	Congratulations! We are done. Select SwiftyIrohaExample target, choose iPhone simulator device and build the application to make sure we have done everything correctly:

.. image:: https://github.com/hyperledger/iroha/raw/develop/docs/image_assets/iroha_swift_guide/iroha_swift_guide_005.png

Before we launch the application and test it we should deploy Iroha peer on our local machine and launch it.

There is good news - steps 1-18 should not be done manually every time - here is the script which does it automatically.

The script for iOS client installation and setup
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

All you need now is to download `build script <https://github.com/hyperledger/iroha/blob/develop/shared_model/packages/ios/iroha-preparation.sh>`__
``iroha-preparation.sh`` and launch it from ``path/to/your/folder/for/example/iroha-ios/project/``.

Starting Iroha Node
^^^^^^^^^^^^^^^^^^^

To run this example, you need an Iroha node up and running. Please check out
:ref:`getting-started` if you want to learn how to start it.

Launching Iroha iOS sample
^^^^^^^^^^^^^^^^^^^^^^^^^^

Now it's time to switch back to SwiftyIrohaSample application and launch it on the simulator. Open Xcode project, select proper sample target and run.
The sample will send test transaction to our node and query the result from blockchain. Successful operations will look similar to this Xcode console output:

.. image:: https://github.com/hyperledger/iroha/raw/develop/docs/image_assets/iroha_swift_guide/iroha_swift_guide_007.png

The output from Iroha terminal window (where the node is running):

.. image:: https://github.com/hyperledger/iroha/raw/develop/docs/image_assets/iroha_swift_guide/iroha_swift_guide_008.png

Great! We have sent our transaction and verified its presence in blockchain.
