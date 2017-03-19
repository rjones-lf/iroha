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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>


#include <string>
#include <iostream>
#include <fstream>


#include <crypto/hash.hpp>

#include <exception>
#include <future>
#include <thread>
#include <chrono>
#include <vector>
#include <random>

namespace tools {

  namespace make_test_peer_status {

    using std::string;
    string filename = "";
    const int must_match_zero = 2;//決め打ち
    const int all_parallel_calc = 10;//並行実行する数


    void do_worker(int a) {
      for (int i = 0; i < 10000000 * a; i++);
      std::cout << a << " end!!" << std::endl;
    }

    void wait_to_get_success_hash(int &flag, string data) {
      flag = false;
      std::mt19937 mt;            // メルセンヌ・ツイスタの32ビット版
      std::random_device rnd;     // 非決定的な乱数生成器
      mt.seed(rnd());            // シード指定
      for (;;) {
        string prehash = data + std::to_string(mt()) + std::to_string(mt());
        string reshash = hash::sha3_512_hex(prehash);
        if (reshash.substr(0, must_match_zero) == string("0", must_match_zero)) break;
//                string reshash = std::to_string( mt() ) + prehash;
//                if( reshash.substr(1, must_match_zero ) == string("0", must_match_zero ) ) break;
      }
      std::cout << data + " end !! " << std::endl;
      flag = true;
    }

    double test_make_hash(int argc, char *argv[]) {
      std::chrono::system_clock::time_point start, end; // 型は auto で可
      start = std::chrono::system_clock::now(); // 計測開始時間

      std::vector<std::thread> thes;
      std::vector<int> flags(all_parallel_calc);

      try {

        for (int i = 0; i < all_parallel_calc; i++) {
          thes.push_back(std::thread(wait_to_get_success_hash, std::ref(flags[i]), std::to_string(i)));
        }

        for (;;) {
          int count_of_accept = 0;
          for (int i = 0; i < all_parallel_calc; i++)
            if (flags[i]) count_of_accept++;
          if (count_of_accept >= all_parallel_calc / 2) break;
        }

        for (int i = 0; i < all_parallel_calc; i++)
          thes[i].detach();

      } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
      }

      end = std::chrono::system_clock::now();  // 計測終了時間

      return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); //処理に要した時間をミリ秒に変換
    }

  }
}

int main(int argc, char *argv[]) {
  double elaplsed_time = tools::make_test_peer_status::test_make_hash(argc, argv);


  std::cout << "============" << std::endl;
  std::cout << "output: " << tools::make_test_peer_status::filename << std::endl;
  if (tools::make_test_peer_status::filename != "") {
    std::ofstream ofs;
    ofs.open(tools::make_test_peer_status::filename);
    ofs << "elaplsed_time : ";
    ofs << elaplsed_time << std::endl;
    ofs.close();
  } else {
    std::cout << "elapsed_time : ";
    std::cout << elaplsed_time << std::endl;
  }
  return 0;
}
//

