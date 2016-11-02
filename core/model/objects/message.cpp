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

#include "message.hpp"

namespace message {

    Message::Message(
            std::string text
    ):
            text(text)
    {}

    std::string Message::getAsJSON(){
        return
            "{\"text\",\""+text+"\"}";
    }

    json_parse::Object Message::getJsonParseRule() {
        json_parse::Object obj = json_parse::Object(json_parse::Type::DICT);
        //obj.dictSub["text"] =  Object(json_parse::Type::STR, text);
        return obj;
    }

}
