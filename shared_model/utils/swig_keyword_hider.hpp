/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef IROHA_SWIG_KEYWORD_HIDER_HPP
#define IROHA_SWIG_KEYWORD_HIDER_HPP

/**
 * Swig doesn't support annotations so we use this trick to use
 * define directive instead of annotation. When we build Swig bindings
 * we undef DEPRECATED in .i file.
 * Also Swig has no idea about final keyword.
 */
#define DEPRECATED [[deprecated]]

#define FINAL final

#endif  // IROHA_SWIG_KEYWORD_HIDER_HPP
