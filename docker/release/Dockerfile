FROM ubuntu:16.04

RUN apt-get update; \
    apt-get install -y libc-ares-dev jq

COPY run-iroha.sh /run-iroha.sh

#Install iroha
COPY iroha.deb /tmp/iroha.deb
RUN apt-get install -y /tmp/iroha.deb; \
    rm -f /tmp/iroha.deb; \
    chmod +x /run-iroha.sh

WORKDIR /opt/iroha_data

CMD ["/run-iroha.sh"]
