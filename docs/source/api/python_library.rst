Python library
==============


About Nexus
---------------
https://nexus.soramitsu.co.jp/ - our new PyPI, Maven, artifacts server with precompilled librares.
You can download binaries without compilation on the client side. It helps to save you time and disk space for installing dependencies like libboost, libz, etc.

Now our packages are distributed only for ``Linux x86_64``, ``MacOS x86_64`` and ``Windows x86_64`` with ``Python 2.7.x`` and ``Python 3.5.x``.

Requirements

.. code-block:: shell

    protobuf >= 3.5.x
    grpcio >= 1.12.x
    python 2.7.x or 3.5.x


Additional Linux requirements

.. code-block:: shell

    Ubuntu-based system (we build packages on Ubuntu 16.04 x86_64)
    libboost 1.66


https://nexus.soramitsu.co.jp/repository/pypi-develop/ - developer wheels (builds from ``develop`` branch)
https://nexus.soramitsu.co.jp/repository/pypi-release/ - release wheels (builds from ``master`` branch)
https://nexus.soramitsu.co.jp/repository/pypi-develop-nightly/ - dev nightly wheels (daily builds from ``develop`` branch)


Pulling from PyPI
---------------

You can download our PyPI package from our Nexus server (instead of pypi.org). To do this, please specify ``-i`` parameter:

From develop

.. code-block:: shell

    pip install -i https://nexus.soramitsu.co.jp/repository/pypi-develop/simple iroha-hl


From release

.. code-block:: shell

    pip install -i https://nexus.soramitsu.co.jp/repository/pypi-release/simple iroha-hl


From develop-nightly

.. code-block:: shell

    pip install -i https://nexus.soramitsu.co.jp/repository/pypi-develop-nightly/simple iroha-hl


Now you can use library, just write ``import iroha``


Pushing to PyPI
---------------

This method allows pushing to repo (only for contributors). Twine package is required.

To develop

.. code-block:: shell

    twine upload --repository-url https://nexus.soramitsu.co.jp/repository/pypi-upload-develop/ /path/to/iroha/wheel/iroha.whl


To release

.. code-block:: shell

    twine upload --repository-url https://nexus.soramitsu.co.jp/repository/pypi-upload-release/ /path/to/iroha/wheel/iroha.whl


To develop-nightly

.. code-block:: shell

    twine upload --repository-url https://nexus.soramitsu.co.jp/repository/pypi-upload-develop-nightly/ /path/to/iroha/wheel/iroha.whl

Adding iroha PyPI repositories
---------------
You can simply add our Nexus server of Python packages distribution. 
For this purpose, create file ``.pypirc`` in your home directory:

.. code-block:: shell

    touch ~/.pypirc

Then paste code below:

.. code-block:: shell

    [distutils]
    index-servers=
        iroha-develop
        iroha-release
        iroha-develop-nightly

    [iroha-develop]
    repository: https://nexus.soramitsu.co.jp/repository/pypi-develop/simple
    [iroha-release]
    repository: https://nexus.soramitsu.co.jp/repository/pypi-release/simple
    [iroha-develop-nightly]
    repository: https://nexus.soramitsu.co.jp/repository/pypi-develop-nightly/simple

Save it and exit. Now you can download packages by this command:

.. code-block:: shell

    pip install -r iroha-develop iroha-hl

And push it: 

.. code-block:: shell

    twine upload -r iroha-develop iroha-hl

Where ``iroha-develop`` - repository name
