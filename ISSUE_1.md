# Issue- Inter-ledger transactions between Fabric and Iroha #159

I do believe that we have to design a common protocol for the Hyperledger Fabric and Iroha. The idea of 2 phase commit seems relevant as we could connect the two ledgers through a common connector which could be linked through the same set of checks. Moreover, Hyperledger Fabric offers a modular architecture for the network designers to plug in their preferred implementations for the components.
There would a common business network for both of them and each main chain operates one or multiple applications/solutions validated by the same group of organizations. Separate Confidential chains should  be built for each of them.

There won't be any need for multiple signatures for making a transaction as there would be confdential chaincode transaction where Validators will only be able to decrypt it. There are different sets of Peers, Proxy Peers, Endorsers and Validators for the peer review of any transactions. Hence, we could think of inter ledger transactions in this way.

Inter-chain transactions and Inter Network transactions can be  made possible between two main networks and between two confidential chains respectively. Chaincodes in a confidential chain can trigger transactions on one or multiple main chain(s). 


