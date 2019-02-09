import os
import time
import binascii

from locust import Locust, TaskSet, events, task

import grpc.experimental.gevent as grpc_gevent
grpc_gevent.init_gevent()

import grpc
from irohalib import Iroha, IrohaGrpc
from irohalib import IrohaCrypto as ic


HOSTNAME = os.environ['HOSTNAME']

class IrohaClient(IrohaGrpc):
    """
    Simple, sample Iroha gRPC client implementation that wraps IrohaGrpc and 
    fires locust events on request_success and request_failure, so that all requests 
    gets tracked in locust's statistics.
    """
    def send_tx_await(self, transaction):
        """
        Send a transaction to Iroha and wait for the final status to be reported in status stream
        :param transaction: protobuf Transaction
        :return: None
        """
        start_time = time.time()
        try:
            tx_future = self._command_service_stub.Torii.future(transaction)
            tx_status = 'NOT_RECEIVED'
            while tx_status not in ['COMMITTED', 'REJECTED']:
                for status in self.tx_status_stream(transaction):
                    tx_status = status[0]
        except grpc.RpcError as e:
            total_time = int((time.time() - start_time) * 1000)
            events.request_failure.fire(request_type="grpc", name='send_tx_await', response_time=total_time, exception=e)
        else:
            total_time = int((time.time() - start_time) * 1000)
            events.request_success.fire(request_type="grpc", name='send_tx_await', response_time=total_time, response_length=0)
            # In this example, I've hardcoded response_length=0. If we would want the response length to be 
            # reported correctly in the statistics, we would probably need to hook in at a lower level


class IrohaLocust(Locust):
    """
    This is the abstract Locust class which should be subclassed. It provides an Iroha gRPC client
    that can be used to make gRPC requests that will be tracked in Locust's statistics.
    """
    def __init__(self, *args, **kwargs):
        super(IrohaLocust, self).__init__(*args, **kwargs)
        self.client = IrohaClient(self.host)


class ApiUser(IrohaLocust):
    
    host = "127.0.0.1:50051"
    min_wait = 100
    max_wait = 1000
    
    class task_set(TaskSet):
        @task
        def send_tx(self):
            iroha = Iroha('admin@test')
            admin_private_key = 'b60e30b7af0f0d233473ca8afe10084c321abc254a911b68c7bb49e924c579bb'

            tx = iroha.transaction([iroha.command(
                'TransferAsset', src_account_id='admin@test', dest_account_id='test@test', asset_id='coin#test',
                amount='0.01', description=HOSTNAME
            )])
            ic.sign_transaction(tx, admin_private_key)
            self.client.send_tx_await(tx)
