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

#include <crypto/base64.hpp>

#include <iostream>
#include <gtest/gtest.h>

template<typename T>
  std::unique_ptr<T[]> vector2UnsignedCharPointer(
  std::vector<T> vec
){
  std::unique_ptr<T[]> res(new T[sizeof(T)*vec.size()+1]);
  size_t pos = 0;
  for(auto c : vec){
    res.get()[pos] = c;
    pos++;
  }
  res.get()[pos] = '\0';
  return res;
}

template<typename T>
std::vector<T> pointer2Vector(
  T* array,
  size_t length
){
    std::vector<T> res(length);
    res.assign(array,array+length);
    return res;
}

void test_text_equals_original_text(unsigned char* text, int text_length){
  ASSERT_STREQ(
    (char*)text,
    (char*)(
      vector2UnsignedCharPointer(
        base64::decode(
          base64::encode(
            pointer2Vector(text, text_length)
          )
        )
      ).get()
    )
  );
}

TEST(Base64, EncodeAndDecodeNormalText){
    unsigned char* original = (unsigned char*)"Hey, I'm mizuki";
    int original_text_length = strlen((char*)original);
    test_text_equals_original_text(original, original_text_length);
}

TEST(Base64, EncodeAndDecodeJapaneseTextWithNoNL){
    unsigned char* original = (unsigned char*)"ソラミツ株式会社";
    int original_text_length = strlen((char*)original);
    test_text_equals_original_text(original, original_text_length);
}

TEST(Base64, EncodeAndDecodeJapaneseTextWithNL){
    unsigned char* original = (unsigned char*)"ご注文は\n分散台帳ですか？";
    int original_text_length = strlen((char*)original);
    test_text_equals_original_text(original, original_text_length);
}

TEST(Base64, EncodeAndDecodeLineText){
  unsigned char* original = (unsigned char*)"\n \
  \n \
  \n \
  ";
  int original_text_length = strlen((char*)original);
  test_text_equals_original_text(original, original_text_length);
}

TEST(Base64, EncodeAndDecodeJapaneseText){
    unsigned char* original = (unsigned char*)"ソラミツ株式会社以呂波耳本へ止千利奴流乎和加餘多連曽津祢那良牟有為能於久耶万計不己衣天阿佐伎喩女美之恵比毛勢須";
    int original_text_length = strlen((char*)original);
    test_text_equals_original_text(original, original_text_length);
}

TEST(Base64, EncodeAndDecodeEmptyText){
  unsigned char* original = (unsigned char*)"";
  int original_text_length = strlen((char*)original);
  test_text_equals_original_text(original, original_text_length);
}

TEST(Base64, EncodeAndDecodeSpace){
  unsigned char* original = (unsigned char*)"                  ";
  int original_text_length = strlen((char*)original);
  test_text_equals_original_text(original, original_text_length);
}


TEST(Base64, EncodeAndDecodeBin){
    unsigned char* original = (unsigned char*)"\xFF\x00\xFF\xFF\xFF";
    int original_text_length = 6;
    test_text_equals_original_text(original, original_text_length);
}

TEST(Base64, EncodeAndDecodeJapanese){
    unsigned char* original = (unsigned char*)"水樹素子";
    int original_text_length = strlen((char*)original);
    test_text_equals_original_text(original, original_text_length);
}


TEST(Base64, EncodeAndDecodeLongText){
    unsigned char* original = (unsigned char*)"AAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    int original_text_length = strlen((char*)original);
    test_text_equals_original_text(original, original_text_length);
}

TEST( Base64, HennaMozi){
  unsigned char* original = (unsigned char*)"à6 L¥õDZÛƒË¸%È] ¬ç”Ã,Ñ ¹š+˜g'Z¡È Ò  ";
  int original_text_length = strlen((char*)original);
  test_text_equals_original_text(original, original_text_length);
}
