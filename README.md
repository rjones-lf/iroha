# What is Hyperledger Iroha?

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/960/badge)](https://bestpractices.coreinfrastructure.org/projects/960)
[![codecov](https://codecov.io/gh/hyperledger/iroha/branch/master/graph/badge.svg)](https://codecov.io/gh/hyperledger/iroha)
[![Snap Status](https://build.snapcraft.io/badge/hyperledger/iroha.svg)](https://build.snapcraft.io/user/hyperledger/iroha)
[![Build Status](https://jenkins.soramitsu.co.jp/buildStatus/icon?job=iroha/iroha-hyperledger/master)](https://jenkins.soramitsu.co.jp/job/iroha/job/iroha-hyperledger/job/master/)
[![Throughput Graph](https://graphs.waffle.io/hyperledger/iroha/throughput.svg)](https://waffle.io/hyperledger/iroha/metrics/throughput)

Blockchain platform Hyperledger Iroha is designed for simple creation and management of assets. This is a distributed ledger of transactions.

Check [overview](http://iroha.readthedocs.io/en/latest/overview.html) page of our documentation.

<img height="300px" src="docs/image_assets/Iroha_3_sm.png"
 alt="Iroha logo" title="Iroha" align="right" />

Iroha has the following features:
1. Creation and management of custom complex assets, such as currencies or indivisible rights, serial numbers, patents, etc.
2. Management of user accounts
3. Taxonomy of accounts based on _domains_ — or _sub-ledgers_ in the system
4. The system of rights and verification of user permissions for the execution of transactions and queries in the system
5. Validation of business rules for transactions and queries in the system

Among the non-functional requirements can be noted a high degree of network fault tolerance _(Byzantine Fault Tolerant)_.

## Documentation

Our documentation is hosted at ReadTheDocs service here: [http://iroha.readthedocs.io](http://iroha.readthedocs.io/en/).
We have documentation in several languages available and you are welcome to contribute on [POEditor website](https://poeditor.com/join/project/SFpZw7o33o)!

### How to explore Iroha really fast?

Check [getting started](http://iroha.readthedocs.io/en/latest/getting_started/) section in your version of localized docs to start exploring the system.

### How to build Iroha?

Use [build guide](http://iroha.readthedocs.io/en/latest/guides/build.html), which might be helpful if you want to modify the code and contribute.

### Is there SDK available?

Yes, in [Java](http://iroha.readthedocs.io/en/latest/guides/libraries/java.html), [Python](http://iroha.readthedocs.io/en/latest/guides/libraries/python.html), [Javascript](http://iroha.readthedocs.io/en/latest/guides/libraries/nodejs.html), builds for [Android](http://iroha.readthedocs.io/en/latest/guides/libraries/android.html), and [iOS](http://iroha.readthedocs.io/en/latest/guides/libraries/swift_ios.html).

### Are there any example applications?

[Android point app](https://github.com/soramitsu/iroha-demo-android) and [JavaScript wallet](https://github.com/soramitsu/iroha-wallet-js).

## Need help?

* Join [telegram chat](https://t.me/hyperledgeriroha) where the maintainers team is able to help you
* Communicate in Gitter chat with our development community [![Join the chat at https://gitter.im/hyperledger-iroha/Lobby](https://badges.gitter.im/hyperledger-iroha/Lobby.svg)](https://gitter.im/hyperledger-iroha/Lobby)
* Submit issues via GitHub Iroha repository
* Join [Hyperledger RocketChat](https://chat.hyperledger.org) #iroha channel to discuss your concerns and proposals
* Use mailing list to spread your word within Iroha development community [hyperledger-iroha@lists.hyperledger.org](mailto:hyperledger-iroha@lists.hyperledger.org)

## License

Iroha codebase is licensed under the Apache License,
Version 2.0 (the "License"); you may not use this file except
in compliance with the License. You may obtain a copy of the
License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Iroha documentation files are made available under the Creative Commons
Attribution 4.0 International License (CC-BY-4.0), available at
http://creativecommons.org/licenses/by/4.0/
