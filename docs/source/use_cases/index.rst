Iroha applications 
===================

Distributed ledger technology, and blockchain specifically,  receive massive attention in recent years. 
This area is constantly evolving with many various applications popping up every day. 
Although, we see a massive adoption is it still far from widespread use. 

Current blockchain system lack of governance and usage simplicity.  
While the technology itself showed to be a disruptive and promising, some basic functionality and lack of clear and simple interface complicates the adoption of technology. 
Iroha team carefully studied all current approaches for creating the best solution according to the needs of clients. 


The main focus of the Iroha is to give soft transition of current centralized applications to multiple application in a shared decentralized space while maintaining the best of two worlds.   
Iroha does not completely removes intermediaries, but rather disintermediate current business process. 
Such disintermediation will benefit both users and service providers enchanting business with new channels and information. (weak)   
Iroha puts client and consumer as a priority. 
Iroha differs from other similar systems in following: 
- Possibility to create a light client tracing only related transactions It is possible to make a lightweight client storing only related transactions.   
- Clear Transaction/Query separation with embedded permissions and role-based access control 
- Native support for mobile client 
- Fast and robust  asynchronous consensus 
- Possibility to tune Iroha to your needs 
- Narrowed attack space due to the restriction of smart contracts to secure core commands
- Possibility to implement middleware software for your application   


Iroha has an embedded role-based control system with rich permissions set.
Each command or query in Iroha is verified according to the defined business rules.   
Permissions in Iroha allow restricting data usage only to a limited set of users reducing the probability of privacy leakage. 
Moreover, Iroha users can store either encrypted data or even only hash for the data integrity.  

Each asset in Iroha has a specific verified creator. 
The asset creation and distribution process are totally transparent for the verifiers. 
We simplified asset creation to just only one command, allowing to tokenize physical items and transfer ownership to other Iroha users. 
 

Iroha uses SWIG to connect interfaces written C++ with a variety of high-level programming languages. 
We flatten the learning curve for developing applications on top of Iroha.  The developer must only learn to use basic Iroha commands to develop a decentralized application with Iroha platform. 
We hide all blockchain abstractions to enable developer concentrate on creating applications as in typical client-server, without bothering much about blockchain specifics.   


Iroha comes as multi-domain blockchain system. Each domain in the system serves as a different subchain.   
This approach allows creating private or public applications with the possibility to securely interact between different domains.  


Use case scenarios
==================
We list some number of abstract use cases and specific advantages that Iroha can introduce to this application. We hope that the applications and use cases will inspire developers and creators to further innovation with Hyperledger Iroha.  


Certificates in Education, Healthcare 
---------------------------- 

Hyperledger Iroha incorporates into the system multiple certifying authorities such universities, schools and medical institutions. 
The flexible permission model used in Hyperledger Iroha allows building certifying identities and grant certificates. 
The storage of explicit and implicit information in users' account build various reputation and identity systems utilizing information from different sources and context.   

Using Hyperledger Iroha each education or medical certificate can be verified that it was given by certification authorities. Immutability and clear validation rules provide transparency to health and education significantly reducing the usage of fake certificates.  

Cross-border asset transfers
----------------------------

Hyperledger Iroha provides fast and clear trade and settlement rules using multi-signature accounts and atomic exchange.   
Asset management is easy as in centralized systems while providing necessary security guarantees.   
 
By simplifying the rules and commands need to create and transfer assets, we lower the barrier to entry, while at the same time maintaining high-security guarantees.  

Financial applications 
----------------------------

Hyperleger Iroha can be very useful in the auditing process. Each information in the system has defined access rules. These access rules can be defined at different levels: user-level, domain-level or system-level.  At the user-level privacy rules for a specific individual are defined. If access rules are defined determined at domain or system level, they are affecting all user in the domain. In Hyperledger Iroha we provide convenient role-based access control, where each role has specific permissions. 

For example, a role of the auditor can have permissions to access the information of a specific user in the domain without bothering the user. To reduce the probability of account hijacking and prevent the auditor from sending malicious queries, the auditor is typically defined as a multi-signature account, meaning that auditor can make queries only having signatures from multiple separate identities.    

Transactions can be traced with a with a local database. Using Iroha-API auditor can query and perform analytics on the data,  execute specific audit software. Hyperledger Iroha supports different scenarios for deploying analytics software:  on a local computer, or execute code on specific middleware.  
This approach allows analyzing Big Data application with Hadoop, Apache etc.  Hypeledger Iroha serves as a guarantor of data integrity and privacy (due to the query permission restriction). 


Identity management
-------------------

Hyperledger Iroha has an intrinsic support for identity management. Each user in the system has a uniquely identified account with personal information, and each transaction is signed and associated by a certain user.  This makes Hyperledger Iroha perfect for various application with KYC (Know Your Customer). 
For example, insurance companies can benefit from querying the information of user’s transaction without worrying about the information truthfulness.  The user can benefit from storing personal information on a blockchain too since authenticated information will reduce the time of claims processing.   a

Hard money loan is the are that can also benefit from storing information on a blockchain. With the property and owner encoded in the blockchain, the processing time of hard money loan will take only a few seconds.   


Supply Chain
----------------------------

Governance of a decentralized system and representing legal rules as a system’s code makes the essential combination of any supply chain system. 
Certification system used in Hyperledger Iroha allows tokenization of physical items and embedding them into the system.  
Each item is incorporated with the information about “what, when, where and why”. 

Permission systems and restricted set of secure core commands narrows the attack vector and provides effortlessly a basic level of privacy. Each transaction is traceable within a system with a hash value, or by the credentials, certificates of the creator. 
 