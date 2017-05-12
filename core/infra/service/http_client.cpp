/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <netdb.h>
#include <unistd.h>
#include <ctime>

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <json.hpp>

#include <infra/service/http_client.hpp>

#include <utils/logger.hpp>

namespace http_client {

const std::string current() {
  time_t t;
  char timestr[256];
  time(&t);
  strftime(timestr, 255, "%a, %d %b %Y %H:%M:%S %Z", localtime(&t));
  return std::string(timestr);
}

std::vector<std::string> split(const std::string& str, char splitter = '\n',
                               unsigned int time = 0) {
  std::vector<std::string> res;
  std::stringstream ss(str);
  std::string tmp;
  unsigned int count = 0;
  while (getline(ss, tmp, splitter)) {
    if (!tmp.empty()) {
      res.push_back(tmp);
    }
    if (time != 0 && count >= time) {
      res.push_back(ss.str());
      break;
    }
    count++;
  }
  return res;
}

std::string trim(const std::string& str) {
  std::string::size_type stIndex = str.find_first_not_of(" \t");
  if (stIndex == std::string::npos) {
    return "";
  }
  return str.substr(stIndex, str.find_last_not_of(" \t") + 1 - stIndex);
}

Request::Request(std::string&& aMethod, std::string&& aPath,
                 std::string&& abody)
    : method(move(aMethod)),
      path(move(aPath)),
      protocol("HTTP/1.1"),
      body(move(abody)) {}

void Request::addHost(std::string host) { this->host = host; }

void Request::addHeader(const std::string& key, std::string&& value) {
  headerset[key] = move(value);
}

void Request::addParams(const std::string& key, std::string&& value) {
  paramset[key] = move(value);
}

const std::string Request::dump() {
  std::string res = method + " " + path + " " + protocol + "\n";
  res += "Host: " + host + "\n";
  for (const auto& it : headerset) {
    res += it.first + ": " + it.second + "\n";
  }
  if (!body.empty()) {
    res += "Content-Length: " + std::to_string(body.size()) + "\n";
  }
  res += "User-Agent: iroha\n";

  res += "\n";
  if (!body.empty()) {
    res += body + "\n";
  }
  return res;
}

using nlohmann::json;

int BUFFER_LENGTH = 2048;

std::tuple<int, std::string> request(std::string dest, int port, Request req) {
  req.addHost(dest);

  struct hostent* hostent;
  struct sockaddr_in server;

  int fd;
  char buffer[BUFFER_LENGTH];

  hostent = gethostbyname(dest.c_str()); /* lookup IP */
  if (hostent == nullptr) {
    logger::error("HttpClient") << "Cannot resolve [" << dest << "]";
    return std::make_tuple(1, "Cannot resolve [" + dest + "]");
  }
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  bcopy(hostent->h_addr, &server.sin_addr, hostent->h_length);
  server.sin_port = htons(port);

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    logger::error("HttpClient") << "Socker error";
    close(fd);
    return std::make_tuple(1, "Socker error");
  }
  if (connect(fd, (struct sockaddr*)&server, sizeof(server)) == -1) {
    logger::error("HttpClient") << "Connection failed";
    close(fd);
    return std::make_tuple(1, "Connection failed");
  }

  // -- SSL --
  SSL_load_error_strings();
  SSL_library_init();
  auto context = SSL_CTX_new(SSLv23_client_method());
  auto ssl = SSL_new(context);

  if (SSL_set_fd(ssl, fd) == 0) {
    ERR_print_errors_fp(stderr);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(context);
    ERR_free_strings();
    close(fd);
    return std::make_tuple(1, "SSL set fd error");
  }
  if (SSL_connect(ssl) != 1) {
    ERR_print_errors_fp(stderr);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(context);
    ERR_free_strings();
    close(fd);
    return std::make_tuple(1, "SSL connect error");
  }

  auto message = req.dump();
  logger::info("HttpClient") << "message \n" << message;
  SSL_write(ssl, message.c_str(), message.size());

  int read_len;
  int isHeader = true;
  std::string res = "";

  read_len = SSL_read(ssl, buffer, BUFFER_LENGTH);

  res += buffer;

  // ======= Header parse =======
  auto lines = split(buffer);
  if (lines.empty()) {
    return std::make_tuple(1, "Response is empty");
  }
  // Remove response top [ HTTP/1.1 200 OK ]
  lines.erase(lines.begin());
  for (auto hp : lines) {
    auto hkv = split(hp, ':', 1);

    // WIP currently, I need body length.
    if (hkv[0] == "Content-Length") {
      BUFFER_LENGTH = std::stoi(trim(hkv[1]));
    }
  }
  // ======= Header parse =======

  // ======= Body  =======
  read_len = SSL_read(ssl, buffer, BUFFER_LENGTH);
  res += buffer;

  // -- SSL --
  SSL_shutdown(ssl);
  SSL_free(ssl);
  SSL_CTX_free(context);
  ERR_free_strings();
  // ---------
  close(fd);

  return std::make_tuple(0, res);
}
}