いろは構築手順 / Iroha build method
===================================

環境 / Environment
==================

今回は"sudo"を書きたくないのでrootで行いましたが普通のユーザーの方がいいと思います。よしなにお願いします


OS
--

::

    root@mizuki # lsb_release -a
    No LSB modules are available.
    Distributor ID: Ubuntu
    Description:  Ubuntu 16.04.1 LTS
    Release:  16.04
    Codename: xenial

Compiler & Maketool
-------------------

基本的にC++14がコンパイルできる物が必要

You need C++14 compilable compiler

::

    root@mizuki # g++ -v
    gcc version 5.4.1 20160904 (Ubuntu 5.4.1-2ubuntu1~16.04)

::

    root@mizuki # cmake  -version
    cmake version 2.8.0

依存ライブラリのインストール / install dependencies
========================================================

::

    # apt -y install build-essential wget xsltproc autoconf automake libtool shtool unzip libssl-dev



java のインストール / Install java
----------------------------------
javaが必要
::

    (ubuntuの場合)
    apt-get install default-jdk

Iroha本体のclone / Clone iroha
==============================

いろはを構築したい場所をよしなに決めてください

Could you decide place you will install iroha as you think best.

::

    ~ # git clone https://github.com/hyperledger/iroha.git
    ~ # ls -l
    total 4
    drwxr-xr-x 8 root root 4096 Dec  8 17:15 iroha

IROHA\_HOMEの設定 / set IROHA\_HOME
-----------------------------------

::

    ~ # cd iroha
    ~/iroha # export IROHA_HOME=$(pwd)
    ~/iroha # echo $IROHA_HOME
    /root/iroha


Iroha本体のbuild / build iroha
==============================

::

    ~/iroha # mkdir build
    ~/iroha # cd build/
    ~/iroha/build # cmake ..
    ~/iroha/build # make
    .
    .
    .
    [ 98%] Built target sumeragi_test
    [100%] Built target iroha-main
    root@mizuki ~/iroha/build#

完成！ Complete!
