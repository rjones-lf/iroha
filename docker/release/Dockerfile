FROM ubuntu:16.04


ARG node

RUN apt-get update;\
    apt-get upgrade; \
    apt-get install -y \
        libc-ares-dev \
        gdebi-core \
        libboost-all-dev

#Install iroha
COPY iroha.deb /tmp/iroha.deb
RUN gdebi --non-interactive /tmp/iroha.deb

WORKDIR /opt/iroha_data

COPY entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["/sbin/init"]
