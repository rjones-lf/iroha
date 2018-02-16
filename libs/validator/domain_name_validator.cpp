/**
 * Copyright AltPlus Inc., Ltd. 2017 All Rights Reserved.
 * http://en.altplus.co.jp
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

#include <boost/spirit/include/qi.hpp>

#include "validator/domain_name_validator.hpp"

namespace validator {
  bool isValidDomainName(const std::string &str) {
    namespace qi = boost::spirit::qi;

    qi::rule<decltype(str.begin())> letter, let_dig, end_with_let_dig,
        let_dig_hyp, label, domain;
    domain = label % '.';
    // I could not express the grammer [ [ <ldh-str> ] <let-dig> ] with
    // <ldh-str> directly.
    label = letter >> qi::repeat(0, 62)[let_dig_hyp - ('-' >> ('.' | qi::eoi))];
    let_dig_hyp = '-' | let_dig;
    let_dig = letter | qi::digit;
    letter = qi::lower | qi::upper;

    auto f = str.begin();
    auto const result = qi::parse(f, str.end(), domain);
    return result && (f == str.end());
  }
}  // namespace validator
