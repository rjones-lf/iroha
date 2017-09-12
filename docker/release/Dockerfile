FROM ubuntu:16.04

ENV IROHA_HOME=/usr/local/iroha
ENV LD_LIBRARY_PATH=${IROHA_HOME}/lib

COPY iroha ${IROHA_HOME}

RUN apt update && \
    apt upgrade -y && \
    useradd -ms /bin/bash iroha && \
    chown -R iroha:iroha ${IROHA_HOME}

WORKDIR ${IROHA_HOME}

CMD ${IROHA_HOME}/bin/irohad --config config/iroha.conf --genesis_block config/zero.block
