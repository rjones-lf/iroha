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
#ifndef IROHA_HTTP_CLIENT_HPP_HPP
#define IROHA_HTTP_CLIENT_HPP_HPP

#include <string>
#include <unordered_map>

namespace http_client {

class Request {
  std::unordered_map<std::string, std::string> headerset;
  std::unordered_map<std::string, std::string> paramset;

  const std::string method;
  const std::string path;
  const std::string protocol;
  const std::string body;

  std::string host;

 public:
  Request(std::string &&aMethod, std::string &&aPath, std::string &&abody);

  void addHost(std::string host);
  void addHeader(const std::string &key, std::string &&value);
  void addParams(const std::string &key, std::string &&value);
  const std::string dump();
};

std::tuple<int, std::string> request(std::string dest, int port, Request req);
}  // namespace http_client

#endif  // IROHA_HTTP_CLIENT_HPP_HPP
