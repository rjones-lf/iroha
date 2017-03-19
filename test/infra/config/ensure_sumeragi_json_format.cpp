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
#include <gtest/gtest.h>
#include <infra/config/config_format.hpp>

TEST(ensure_sumeragi_json_format, normal_sumeragi_json) {
  ASSERT_TRUE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"name\":\"samari\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"privateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"ip\":\"172.17.0."
          "4\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "5\",\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"publicKey\":\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
}

TEST(ensure_sumeragi_json_format, bad_json) {
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"name\":\"samari\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"privateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"ip\":\"172.17.0."
          "4\",\"name\":\"natori\",\"publicKey\":{},{\"ip\":\"172.17.0.5\","
          "\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"publicKey\":\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
}

TEST(ensure_sumeragi_json_format, bad_ip) {
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"name\":\"samari\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"privateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"ip\":\"172.17.0."
          "4\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "987\",\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"publicKey\":\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"name\":\"samari\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"privateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"ip\":\"172.17.0.4."
          "\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "5\",\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"publicKey\":\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
}

TEST(ensure_sumeragi_json_format, missing_key) {
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"name\":\"samari\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"privateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"iP\":\"172.17.0."
          "4\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "5\",\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"publicKey\":\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"name\":\"samari\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"privateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"ip\":\"172.17.0."
          "4\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "5\",\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"name\":\"samari\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"privateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"ip\":\"172.17.0."
          "4\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "5\",\"name\":\"kabohara\",\"PublicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"publicKey\":\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"name\":\"samari\",\"publicKey\":\"Sht5opDIxbyK+"
          "oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"privateKey\":\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"ip\":\"172.17.0."
          "4\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "5\",\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"publicKey\":\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"myname\":\"samari\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"privateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"ip\":\"172.17.0."
          "4\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "5\",\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"publicKey\":\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"name\":\"samari\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"rivateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"ip\":\"172.17.0."
          "4\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "5\",\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"publicKey\":\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"name\":\"samari\",\"privateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"ip\":\"172.17.0."
          "4\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "5\",\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"publicKey\":\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
}

TEST(ensure_sumeragi_json_format, useless_key) {
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"name\":\"samari\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"privateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\","
          "\"aaaa\":\"hogehoge\"},\"group\":[{\"ip\":\"172.17.0.3\",\"name\":"
          "\"mizuki\",\"publicKey\":\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0="
          "\"},{\"ip\":\"172.17.0.4\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "5\",\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"publicKey\":\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
  ASSERT_FALSE(config::ConfigFormat::getInstance().ensureFormatSumeragi(
      "{\"me\":{\"ip\":\"172.17.0.6\",\"name\":\"samari\",\"publicKey\":"
          "\"Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\",\"privateKey\":"
          "\"aGIuSZRhnGfFyeoKNm/"
          "NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==\"}"
          ",\"group\":[{\"ip\":\"172.17.0.3\",\"name\":\"mizuki\",\"publicKey\":"
          "\"jDQTiJ1dnTSdGH+yuOaPPZIepUj1Xt3hYOvLQTME3V0=\"},{\"ip\":\"172.17.0."
          "4\",\"name\":\"natori\",\"publicKey\":"
          "\"Q5PaQEBPQLALfzYmZyz9P4LmCNfgM5MdN1fOuesw3HY=\"},{\"ip\":\"172.17.0."
          "5\",\"name\":\"kabohara\",\"publicKey\":\"f5MWZUZK9Ga8XywDia68pH1HLY/"
          "Ts0TWBHsxiFDR0ig=\"},{\"ip\":\"172.17.0.6\",\"name\":\"samari\","
          "\"key123456\":\"98765\",\"publicKey\":\"Sht5opDIxbyK+"
          "oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=\"}]}"));
}
