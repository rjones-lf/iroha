Installing Dependencies
=======================

This page contains references and guides about installation of various tools you may need during build of different targets of Iroha project.

.. Note::
	Please note that most likely you do not need to install all the listed tools.
	Some of them are required only for building specific versions of Iroha Client Library.

Automake
--------

Installation on Ubuntu
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: shell

    sudo apt install automake
    automake --version
    # automake (GNU automake) 1.15

Bison
-----

Installation on Ubuntu
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: shell

    sudo apt install bison
    bison --version
    # bison (GNU Bison) 3.0.4

CMake
-----

Minimum required version is 3.8, but we recommend to install the latest available version (3.10.3 at the moment).

Installation on Ubuntu
^^^^^^^^^^^^^^^^^^^^^^

Since Ubuntu repositories contain unsuitable version of CMake, you need to install the new one manually.
Here is how to build and install CMake from sources.

.. code-block:: shell

    wget https://cmake.org/files/v3.10/cmake-3.10.3.tar.gz
    tar -xvzf cmake-3.10.3.tar.gz
    cd cmake-3.10.3/
    ./configure
    make
    sudo make install
    cmake --version
    # cmake version 3.10.3

Git
---

Installation on Ubuntu
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: shell

    sudo apt install git
    git --version
    # git version 2.7.4
