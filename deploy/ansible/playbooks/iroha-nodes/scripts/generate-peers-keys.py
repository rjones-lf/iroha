#!/usr/bin/python3
import argparse
import csv
import binascii
# import sys
import ed25519
import secrets
# import subprocess

class Peer:
    def __init__(self, host, port, priv_key, pub_key):
        self.host = host
        self.port = port
        self.priv_key = priv_key
        self.pub_key = pub_key


# returns a list of [public_key, private_key]
# def generate_keypair():
#     k = subprocess.check_output(['./ed25519-cli', 'keygen']).decode('utf-8').split('\n')
#     return [k[0].split(':')[1].replace(' ', ''), k[1].split(':')[1].replace(' ', '')]

def generate_keypair():
    priv = secrets.token_bytes(32)
    return [binascii.hexlify(ed25519.derive_pubkey_from_priv(priv)), binascii.hexlify(priv)]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("file",
                        help="pass file path to the peers.csv file")
    args = parser.parse_args()

    peers = []
    with open(args.file) as peers_file:
        peersreader = csv.reader(peers_file, delimiter=';')
        for peer in peersreader:
            keys = generate_keypair()
            print(keys)
            peers.append(";".join((peer[0], peer[1], keys[0], keys[1])) + '\n')

    with open(args.file, 'w') as peers_file:
        peers_file.write("host;port;priv_key_b64_encoded;pub_key_b64_encoded\n")
        peers_file.writelines(peers)


if __name__ == "__main__":
    main()
