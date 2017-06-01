/*
Copyright Soramitsu Co., Ltd. 2016,2017 All Rights Reserved.

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
#ifndef IROHA_EXPLORE_H
#define IROHA_EXPLORE_H

#include <utils/logger.hpp>
#include <iomanip>
#include <string>
namespace explore {

    namespace sumeragi{

        struct PrintProgress {
            int MAX;
            PrintProgress():MAX(100){};

            void print(int progressNum, std::string &&msg) {
                logger::explore("sumeragi") << "\033[95m+| " << std::setw(3) << progressNum / MAX << "|+\033[0m" << msg;
            }
        };

        inline void printInfo(std::string&& msg){
            logger::explore("sumeragi") << "\x1b[36m " << msg << " \033[0m";
        }

        inline void initialize(){
            logger::explore("sumeragi") << "\033[95m+==ーーーーーーーーー==+\033[0m";
            logger::explore("sumeragi") << "\033[95m|+-ーーーーーーーーー-+|\033[0m";
            logger::explore("sumeragi") << "\033[95m|| 　　　　　　　　　 ||\033[0m";
            logger::explore("sumeragi") << "\033[95m|| いろは合意形成機構 ||\033[0m";
            logger::explore("sumeragi")
                    << "\033[95m|| 　　　\033[1mすめらぎ\033[0m\033[95m　　 ||\033[0m";
            logger::explore("sumeragi") << "\033[95m|| 　　　　　　　　　 ||\033[0m";
            logger::explore("sumeragi") << "\033[95m|+-ーーーーーーーーー-+|\033[0m";
            logger::explore("sumeragi") << "\033[95m+==ーーーーーーーーー==+\033[0m";
            logger::explore("sumeragi") << "- 起動/setup";
            logger::explore("sumeragi") << "- 初期設定/initialize";
        }

        inline void printJudge(int numValidSignatures, int numValidationPeer, int faulty) {
            std::stringstream resLine[5];
            for (int i = 0; i < numValidationPeer; i++) {
                if (i < numValidSignatures) {
                    resLine[0] << "\033[1m\033[92m+-ー-+\033[0m";
                    resLine[1] << "\033[1m\033[92m| 　 |\033[0m";
                    resLine[2] << "\033[1m\033[92m|-承-|\033[0m";
                    resLine[3] << "\033[1m\033[92m| 　 |\033[0m";
                    resLine[4] << "\033[1m\033[92m+-＝-+\033[0m";
                } else {
                    resLine[0] << "\033[91m+-ー-+\033[0m";
                    resLine[1] << "\033[91m| 　 |\033[0m";
                    resLine[2] << "\033[91m| 否 |\033[0m";
                    resLine[3] << "\033[91m| 　 |\033[0m";
                    resLine[4] << "\033[91m+-＝-+\033[0m";
                }
            }
            for (int i = 0; i < 5; ++i) logger::explore("sumeragi") << resLine[i].str();

            std::string line;
            for (int i = 0; i < numValidationPeer; i++) line += "==＝==";
            logger::explore("sumeragi") << line;

            logger::explore("sumeragi")
                    << "numValidSignatures:" << numValidSignatures << " faulty:" << faulty;
        }

        inline void printAgree() {
            logger::explore("sumeragi") << "\033[1m\033[92m+==ーー==+\033[0m";
            logger::explore("sumeragi") << "\033[1m\033[92m|+-ーー-+|\033[0m";
            logger::explore("sumeragi") << "\033[1m\033[92m|| 承認 ||\033[0m";
            logger::explore("sumeragi") << "\033[1m\033[92m|+-ーー-+|\033[0m";
            logger::explore("sumeragi") << "\033[1m\033[92m+==ーー==+\033[0m";
        }

        inline void printReject() {
            logger::explore("sumeragi") << "\033[91m+==ーー==+\033[0m";
            logger::explore("sumeragi") << "\033[91m|+-ーー-+|\033[0m";
            logger::explore("sumeragi") << "\033[91m|| 否認 ||\033[0m";
            logger::explore("sumeragi") << "\033[91m|+-ーー-+|\033[0m";
            logger::explore("sumeragi") << "\033[91m+==ーー==+\033[0m";
        }
    }
};

#endif //IROHA_EXPLORE_H
