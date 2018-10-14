FROM ubuntu:16.04

RUN apt-get update; \
    apt-get install -y libc-ares-dev jq

COPY wait-for-it.sh /wait-for-it.sh
COPY run-iroha.sh /run-iroha.sh

#Install iroha
COPY iroha.deb /tmp/iroha.deb
RUN apt-get install -y /tmp/iroha.deb; \
    rm -f /tmp/iroha.deb; \
    chmod +x /run-iroha.sh; \
    chmod +x /wait-for-it.sh

WORKDIR /opt/iroha_data

CMD ["/run-iroha.sh"]
