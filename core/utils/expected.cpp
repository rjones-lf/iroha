/*
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * http://soramitsu.co.jp
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *          http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "expected.hpp"

UnexpectedType::UnexpectedType(std::exception_ptr&& excptr) noexcept
    : excptr_(excptr) {}

const std::exception_ptr& UnexpectedType::excptr() const noexcept {
  return excptr_;
}

VoidHandler::VoidHandler() noexcept
    : excptr_(std::make_exception_ptr(exception::None())), valid_(true) {}

VoidHandler::VoidHandler(const UnexpectedType& exc) noexcept
    : excptr_(exc.excptr()), valid_(false) {}

std::string VoidHandler::error() const {
  try {
    std::rethrow_exception(excptr_);
  } catch (const exception::IrohaException& e) {
    return e.message();
  }
}
