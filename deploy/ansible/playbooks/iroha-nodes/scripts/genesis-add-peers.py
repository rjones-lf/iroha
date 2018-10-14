#!/usr/env/python3
import json, csv, sys, base64
'''
peers.csv
host;port;priv_key_b64_encoded;pub_key_b64_encoded
'''

class Peer:
    def __init__(self, host, port, priv_key, pub_key):
        self.host = host
        self.port = port
        self.priv_key = priv_key
        self.pub_key = pub_key

def parse_peers(peers_csv_fp):
    peers = []
    with open(peers_csv_fp) as csvfile:
        peersreader = csv.reader(csvfile, delimiter=';')
        next(peersreader, None) # skip the header
        for peer in peersreader:
            peers.append(Peer(peer[0], peer[1], peer[3], peer[2]))
    return peers

def genesis_add_peers(peers_list, genesis_block_fp):
    with open(genesis_block_fp, 'r+') as genesis_json:
        genesis_dict = json.load(genesis_json)
        try:
            genesis_dict['payload']['transactions'][0]['payload']['reducedPayload']['commands'] = filter(lambda c: not c.get('addPeer'), genesis_dict['payload']['transactions'][0]['payload']['reducedPayload']['commands'])
        except KeyError:
            pass
        genesis_dict['payload']['transactions'][0]['payload']['reducedPayload']['commands'] = list(genesis_dict['payload']['transactions'][0]['payload']['reducedPayload']['commands'])
        for p in peers_list:
            p_add_command = {"addPeer": {"peer": {"address": "%s:%s" % (p.host, p.port), "peerKey":hex_to_b64(p.pub_key)}}}
            genesis_dict['payload']['transactions'][0]['payload']['reducedPayload']['commands'].append(p_add_command)
        with open("genesis.block", 'w') as genesis_json_dump:
            json.dump(genesis_dict, genesis_json_dump, sort_keys=True)
            genesis_json_dump.truncate()


def hex_to_b64(hex_string):
    hex_string = base64.b64encode(bytearray.fromhex(hex_string))
    return hex_string.decode('utf-8')

def make_keys(peers):
    for i, p in enumerate(peers):
        with open('node%s.priv' % i, 'w+') as priv_key_file:
            priv_key_file.write(p.priv_key)
        with open('node%s.pub' % i, 'w+') as pub_key_file:
            pub_key_file.write(p.pub_key)

if __name__ ==  "__main__":
    command = sys.argv[1]
    peers_csv = sys.argv[2]
    try:
        json_conf = sys.argv[3]
    except IndexError:
        pass
    peers = parse_peers(peers_csv)
    if command == 'add_iroha_peers':
        genesis_add_peers(peers, json_conf)
    elif command == 'make_key_files':
        make_keys(peers)
    else:
        print('Invalid command')