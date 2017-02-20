#!/usr/bin/python3

import json
import socket
import threading
import argparse
from argparse import RawTextHelpFormatter
from threading import Lock



# returns new dict with the same key-value pairs, but without key 'privateKey'
def extract(config):
    return {i:config[i] for i in config if i != 'privateKey'}


# newtork is the array of dicts, which are json-parsed outputs of make_sumeragi
def make_configs(network):
    result = {}
    for id_, config1 in network.items():
        c = {
            "me": config1,
            "group": [extract(config1)]
        }

        for config2 in network.values():
            if config1 == config2:
                continue

            c['group'].append(extract(config2))

        result[id_] = json.dumps(c)

    return result


class ThreadedServer(object):
    def __init__(self, host, port, args):
        self.host = host
        self.port = port
        self.args = args
        self.locks = {}
        self.network = {}
        self.LOCK = Lock()
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind((self.host, self.port))


    def listen(self):
        self.sock.listen(5)
        for _ in range(self.args.number):
            client, address = self.sock.accept()
            tid = client.fileno()
            print("[+] Got connection from {0}".format(address))
            threading.Thread(target = self.listenToClient,args = (tid,client,address)).start()


    def listenToClient(self, tid, client, address):
        data = client.recv(2**20) # 1 MB at max
        if data: # we received something
            print(data)
            try:
                self.LOCK.acquire()
                self.network[tid] = json.loads(data.decode('utf-8'))
                self.locks[tid] = Lock()
                self.LOCK.release()

                while True:
                    if len(self.network) >= self.args.number:
                        for id_ in self.locks:
                            try:
                                # we may try to release unlocked locks
                                self.locks[id_].release()
                            except:
                                # we don't care about it
                                pass
                        break
                    else:
                        self.locks[tid].acquire() # wait until we receive all configs


                # if program is here, then we received all configs
                config = make_configs(self.network)
                client.send(config[tid].encode('utf-8'))

            except Exception as e:
                print("[-] Something went wrong for {0}:\n{1}\n\n".format(address, data))
                print(str(e))
            client.close() # close the connection



if __name__ == "__main__":
    desc = '''
Use this script to generate configs for any number of nodes. 

It creates a TCP server with N threads (you must explicitly specify N), 
each receives output of make_sumeragi program and after receiving of N 
configs, it generates and sends sumeragi.json for each node separately. 

On server (assume it has IP 172.17.0.1):
    python3 config-server.py 4

On each node:
    make_sumeragi -i eth0 -n name -o out.txt
    cat out.txt | nc 172.17.0.1 8000 > sumeragi.json
    '''

    parser = argparse.ArgumentParser(description=desc, formatter_class=RawTextHelpFormatter)
    parser.add_argument('number', metavar='N', type=int,
                        help='number of iroha nodes')

    args = parser.parse_args()

    print(desc)
    print("[+] Waiting for {0} configs".format(args.number))
    HOST = '0.0.0.0'
    PORT = 8000

    ThreadedServer(HOST, PORT, args).listen()
 



