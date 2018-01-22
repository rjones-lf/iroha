/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include <iostream>
#include <rttr/registration>

#include "cryptography/public_key.hpp"

#include <gtest/gtest.h>

/**
 * @brief This file is a reference point to rttr, not just the test.
 *
 * @example Documentation is at
 * http://www.rttr.org/doc/master/five_minute_tutorial_page.html
 *
 * @note If you get
 * terminate called after throwing an instance of
 * '__gnu_cxx::recursive_init_error' what():  std::exception Aborted then try to
 * not use RTTR_ENABLE() RTTR_REGISTRATION_FRIEND
 */

RTTR_REGISTRATION {
  using namespace rttr;
  using namespace shared_model::crypto;
  registration::class_<PublicKey>("shared_model::crypto::PublicKey")
      .constructor<const std::string &>()(policy::ctor::as_object)
      .constructor<const Blob &>()(policy::ctor::as_object)
      .method("toString", &PublicKey::toString)
      .method("copy", &PublicKey::copy);
}

TEST(Reflection, TwoConstructors) {
  using namespace rttr;
  type amount = type::get_by_name("shared_model::crypto::PublicKey");
  ASSERT_EQ(amount.get_constructors().size(), 2);
}

TEST(Reflection, TwoMethods){
  using namespace rttr;
  type amount = type::get_by_name("shared_model::crypto::PublicKey");
  ASSERT_EQ(amount.get_methods().size(), 2);
}

