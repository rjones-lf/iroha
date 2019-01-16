/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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
