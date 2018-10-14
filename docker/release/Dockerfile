FROM ubuntu:16.04

RUN apt-get update; \
    apt-get install -y libc-ares-dev jq

COPY wait-for-it.sh run-iroha.sh go-grpc-health /

#Install iroha
COPY iroha.deb /tmp/iroha.deb
RUN apt-get install -y /tmp/iroha.deb; \
    rm -f /tmp/iroha.deb; \
    chmod +x /run-iroha.sh; \
    chmod +x /wait-for-it.sh; \
    chmod +x /go-grpc-health

WORKDIR /opt/iroha_data

HEALTHCHECK \
  --interval=10s --timeout=3s --retries=6 \
  CMD /go-grpc-health -host localhost -port 50051

CMD ["/run-iroha.sh"]
