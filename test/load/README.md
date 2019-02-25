# Load tests

See [locustfile.py](locustfile.py) for descriptions of task sets implemented using [locust](https://github.com/locustio/locust) framework.

## Prerequisites

 * Docker
 * Docker Compose

## Build steps

Create a Docker image with Python, Iroha, Locust, and gRPC.
```sh
docker build -t iroha-locust .
```

## Running the tests

1. Configure environment via running interactive [update-env.py](update-env.py) script.
   
   **Or manually** specify Iroha node address and port in `TARGET_URL` in environment configuration [file](config.env).

    **Mac hosts** If you are running Iroha on the same host as locust, you most likely need to use `docker.for.mac.localhost:50051`, where `50051` is Torii port.
    **Linux hosts** Accessing host from container is not trivial, please refer to [this](https://github.com/docker/for-linux/issues/264) issue.
   
   Specify desired test script as locustfile in `LOCUSTFILE_PATH` in Compose file (e.g. locustfile.py or locustfile-performance.py)

3. Run locust
    ```sh
    docker-compose up
    ```

    4 slaves can be spawned by adding `--scale locust-slave=4` to the previous command.

4. Access locust web interface as specified in [documentation](https://docs.locust.io/en/stable/quickstart.html#open-up-locust-s-web-interface).
