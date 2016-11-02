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

#include <json.hpp>

#include <iostream>
#include <string>

#include "../../server/http_server.hpp"
#include "../../vendor/Cappuccino/cappuccino.hpp"
  
namespace http {
  
  using nlohmann::json;
  using Request = Cappuccino::Request;
  using Response = Cappuccino::Response;

  void server() {

    Cappuccino::Cappuccino( 0, nullptr);

    Cappuccino::route("/",[](std::shared_ptr<Request> request) -> Response{
        auto res =  Response(request);
        res.file("index.html");
        return res;
    });

    // runnning
    Cappuccino::run();

  }
};  // namespace http
