FROM ubuntu:16.04

ENV IROHA_HOME=/usr/local/iroha
ENV JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
ENV LD_LIBRARY_PATH=${IROHA_HOME}/lib

COPY iroha ${IROHA_HOME}
COPY iroha/opt /opt

RUN apt update && apt upgrade -y; \
    apt install -y default-jdk; \
    rm -fr ${IROHA_HOME}/opt; \
    useradd -ms /bin/bash iroha; \
    chown -R iroha:iroha $IROHA_HOME

USER iroha

WORKDIR ${IROHA_HOME}

CMD ["/usr/local/iroha/bin/iroha-main"]
