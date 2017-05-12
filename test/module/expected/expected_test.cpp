/**
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <memory>
#include <utils/expected.hpp>

#include "sample_generated.h"

TEST(UseExpected, returnsValidData) {
  auto whatsNumber = [](int number) -> Expected<std::string> {
    if (number == 123) {
      return std::string("YES");
    }
    if (number == 321) {
      return makeUnexpected(exception::crypto::InvalidKeyException("Hoge"));
    }
    return makeUnexpected(
        exception::crypto::InvalidMessageLengthException("InvalidLength"));
  };

  auto res = whatsNumber(123);
  if (res) {
    ASSERT_TRUE(res);
    ASSERT_TRUE(res.valid());
    ASSERT_STREQ((*res).c_str(), "YES");
    ASSERT_STREQ(res.value().c_str(), "YES");
  } else {
    FAIL();
  }
}

TEST(UseExpected, returnsException1) {
  auto whatsNumber = [](int number) -> Expected<std::string> {
    if (number == 123) {
      return std::string("YES");
    }
    if (number == 321) {
      return makeUnexpected(exception::crypto::InvalidKeyException("Hoge"));
    }
    return makeUnexpected(
        exception::crypto::InvalidMessageLengthException("Foo"));
  };
  auto res = whatsNumber(321);
  if (res) {
    FAIL() << "Invalid valid()";
  } else {
    try {
      std::rethrow_exception(res.excptr());
    } catch (const exception::crypto::InvalidKeyException& e) {
      ASSERT_STREQ(res.error().c_str(),
                   "<<INSECURE>> Keyfile is invalid, cause is: Hoge");
    } catch (const exception::crypto::InvalidMessageLengthException& e) {
      FAIL() << "exception::crypto::InvalidMessageLengthException";
    } catch (const exception::IrohaException& e) {
      FAIL() << "Unknown exception: " << typeid(decltype(e)).name();
    }
  }
}

TEST(UseExpected, returnsException2) {
  auto whatsNumber = [](int number) -> Expected<std::string> {
    if (number == 123) {
      return std::string("YES");
    }
    if (number == 321) {
      return makeUnexpected(exception::crypto::InvalidKeyException("Hoge"));
    }
    return makeUnexpected(
        exception::crypto::InvalidMessageLengthException("Foo"));
  };
  auto res = whatsNumber(999);
  if (res) {
    FAIL() << "Invalid valid()";
  } else {
    try {
      std::rethrow_exception(res.excptr());
    } catch (const exception::crypto::InvalidKeyException& e) {
      FAIL() << "exception::crypto::InvalidKeyException";
    } catch (const exception::crypto::InvalidMessageLengthException& e) {
      ASSERT_STREQ(res.error().c_str(),
                   "<<INSECURE>> Message Foo has wrong length");
    } catch (const exception::IrohaException& e) {
      FAIL() << "Unknown exception: " << typeid(decltype(e)).name();
    }
  }
}

TEST(UseExpected, VoidHandler) {
  auto voidTest = [](int number) -> VoidHandler {
    if (number != 123) {
      return makeUnexpected(exception::IrohaException("Invalid"));
    }
    return {};
  };
  auto res = voidTest(123);
}

TEST(UseExpected, flatBuffersPtrMove) {
  auto f = [](const char *s, int x, double d) -> Expected<flatbuffers::unique_ptr_t> {
    flatbuffers::FlatBufferBuilder fbb;
    auto sampleOffset = CreateSampleDirect(fbb, s, x, d);
    fbb.Finish(sampleOffset);
    return fbb.ReleaseBufferPointer();
  };

  auto e = f("HOGEHOGE", 12345, 3.14);
  ASSERT_TRUE(e);
  flatbuffers::unique_ptr_t extracted;
  e.move_value(extracted);

  auto root = flatbuffers::GetRoot<Sample>(extracted.get());
  ASSERT_STREQ(root->strvalue()->c_str(), "HOGEHOGE");
  ASSERT_EQ(root->intvalue(), 12345);
  ASSERT_EQ(root->doublevalue(), 3.14);
}
