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

#include "../../server/http_server.hpp"
#include "../../vendor/Cappuccino/cappuccino.hpp"
#include "../../util/logger.hpp"

#include "../../consensus/connection/connection.hpp"
#include "../../consensus/consensus_event.hpp"
#include "../../model/commands/transfer.hpp"
#include "../../model/objects/domain.hpp"
#include "../../model/transaction.hpp"
#include "../../service/json_parse_with_json_nlohman.hpp"
#include "../../service/peer_service.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>

  
namespace http {
  
  using nlohmann::json;
  using Request = Cappuccino::Request;
  using Response = Cappuccino::Response;

  
  template<typename T>
  using Transaction = transaction::Transaction<T>;
  template<typename T>
  using ConsensusEvent = event::ConsensusEvent<T>;
  template<typename T>
  using Add = command::Add<T>;
  template<typename T>
  using Transfer = command::Transfer<T>;

  using Object = json_parse::Object;

  json responseError(std::string message){
    return json({
      {"message", message},
      {"status", 400}
    });
  }

  enum class RequestType{
      Int,
      Str,
      Bool,
      Float
  };

  void server(std::map<std::string,std::function<int(Object)>> apis) {
    logger::info("server", "initialize server!");
    Cappuccino::Cappuccino( 0, nullptr);

    for(const auto api: apis){
        Cappuccino::route( api.first,[api](std::shared_ptr<Request> request) -> Response{
            auto data = request->json();
            auto res = Response(request);
            if(data.empty()) {
              res.json(responseError("Invalied JSON"));
              return res;
            }
            api.second(json_parse_with_json_nlohman::parser::load(data));

            res.json({
                {"message", "OK"},
                {"status", 200}
            });
            return res;
        });
    };

    logger::info("server", "start server!");
    // runnning
    Cappuccino::run();

  }
};  // namespace http
