# How to run network of N iroha nodes?

1. Make sure you have the latest `iroha-docker` image:

   ````
   docker pull hyperledger/iroha-docker
   ````

2. On each node create ***correct*** `sumeragi.json`:

   - double check all ip addresses
   - double check all public keys

   Save it somewhere (we assume it is in `/tmp/sumeragi.json` on each node).

3. Run `iroha-docker` on each node:

   ```bash
   # beware of copy-pasting: change /tmp/sumeragi.json to location of this file in host OS
   docker run \
     -d \
     -p 1204:1204 \
     -it \
     -v /tmp/sumeragi.json:/iroha/config/sumeragi.json \
     hyperledger/iroha-docker
   ```

   ```bash
   # semantics:
   -d                        -- detached (from terminal)
   -p 1204:1204              -- forward port: host 1204 <-> 1204 container
                                docker proxy will be listening on 0.0.0.0:1204 and 
                                it will be forwarding all traffic to the container
   -it                       -- not necessary but help us to get rid of some errors
                                i - keep STDIN open even if not attached
                                t - allocate TTY
   -v /tmp/sumeragi.json:/iroha/config/sumeragi.json 
                             -- mount config from host to container 
                                (semantics: -v from:to, only absolute paths)
   hyperledger/iroha-docker  -- image tag
   ```

   ​

In the end, you have a network of N nodes, you can communicate with them via host IPs. Each node from this moment runs iroha as if you use it without docker.

