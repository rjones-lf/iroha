# Load tests

See [locustfile.py](locustfile.py) for descriptions of task sets implemented using [locust](https://github.com/locustio/locust) framework.

## Prerequisites

 * Docker
 * Docker Compose
 * Python 3

## Build steps

1. Create a Docker image with Python, locust, and gRPC.
    ```sh
    docker build -t iroha-locust .
    ```

2. Copy [irohalib.py](https://github.com/hyperledger/iroha/blob/master/example/python/irohalib.py) and [ed25519.py](https://github.com/hyperledger/iroha/blob/master/example/python/ed25519.py) to the current directory and follow the preparation steps in [libiroha.md](https://github.com/hyperledger/iroha/blob/master/example/python/irohalib.md).

## Running the tests

1. Specify Iroha node address and port in `TARGET_URL` in Compose [file](docker-compose.yml).

    **Note for Mac hosts** If you are running Iroha on the same host as locust, you most likely need to use `docker.for.mac.localhost:50051`, where `50051` is Torii port.

2. Run locust
    ```sh
    docker-compose up
    ```

    4 slaves can be spawned by adding `--scale locust-slave=4` to the previous command.

3. Access locust web interface as specified in [documentation](https://docs.locust.io/en/stable/quickstart.html#open-up-locust-s-web-interface).
