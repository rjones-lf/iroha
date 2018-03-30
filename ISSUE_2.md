Issue- Design of Smart Contracts #249

@Warchant & @Takemiyamakoto proposed a method of using lxc container with any inbuilt language compiler and another one using Scala. I would like to add up something regarding this. 

What I propose is a bit different and I think of working on a development environment which could deploy smart contracts like either of solidity, go or scala. This gives  us the flexibility to the programmers to interact with the standalone client very easily. It can be tried with Ethereum's JSON-RPC API and GO-Ethereum by running geth --rpc.

Disclaimer- I am not done with the codes. I have just thought of this and I don't know whether it's going to work or not. It is just a proposal. 

How to use the command line -

We need to enable JSON-RPC. geth --rpc should be running in the other terminal.

   1.  Define an alias called defaultSender by typing 		BTCAddressAliasSet defaultSender 		   		0x465e79b940bc2157e4259ff6b2d92f454497f1e
   2.  Set the environment variable BTC_SENDER prior to the 	 running development environment. 
   3. Set the JVM System property BTC.sender when running the command line, that is, run xyz (say) -DBTC.sender=0x465e79b940bc2157e4259ff6b2d92f454497f1e4
    At any time, on the SBT command like, run

> set BTCcfgSender := "0x465e79b940bc2157e4259ff6b2d92f454497f1e4"


