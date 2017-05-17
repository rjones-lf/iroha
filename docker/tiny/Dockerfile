FROM ubuntu:16.04

ENV IROHA_HOME=/usr/local/iroha
ENV LD_LIBRARY_PATH=${IROHA_HOME}/lib

COPY iroha ${IROHA_HOME}

RUN apt update && apt upgrade -y; \
    useradd -ms /bin/bash iroha; \
    chown -R iroha:iroha $IROHA_HOME

USER iroha

WORKDIR ${IROHA_HOME}

CMD ["/usr/local/iroha/bin/iroha-main"]
