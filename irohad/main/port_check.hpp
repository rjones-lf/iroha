/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PORT_CHECK_HPP
#define IROHA_PORT_CHECK_HPP

#include <netinet/in.h>
#include <arpa/inet.h>

bool checkTcpPortInUse(std::string address, size_t port) {
    struct sockaddr_in client;
    int sock;

    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = inet_addr(address.c_str());

    sock = (int) socket(AF_INET, SOCK_STREAM, 0);

    int result = connect(sock, (struct sockaddr *) &client,sizeof(client));

    if (result == 0) {
        close(sock);
        return true;
    }
    return false;
}

#endif //IROHA_PORT_CHECK_HPP
