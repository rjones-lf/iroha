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
#include <crypto/signature.hpp>

#include <gtest/gtest.h>

#include <string>

TEST(Signature, E){
  signature::KeyPair keyPair = signature::generateKeyPair();
  std::string nonce = "c0a5cca43b8aa79eb50e3464bc839dd6fd414fae0ddf928ca23dcebf8a8b8dd0";

  std::string signature = signature::sign(
    nonce,
    base64::encode(keyPair.publicKey),
    base64::encode(keyPair.privateKey)
  );

  ASSERT_TRUE(signature::verify(
    signature,
    nonce,
    base64::encode(keyPair.publicKey)));
}

TEST(Signature, keyPair){
  signature::KeyPair keyPair = signature::generateKeyPair();

  std::string nonce = "c0a5cca43b8aa79eb50e3464bc839dd6fd414fae0ddf928ca23dcebf8a8b8dd0";
  std::string signature_b64 = signature::sign(
    nonce,
    keyPair
  );

  std::cout << base64::encode(keyPair.publicKey) << std::endl;
  std::cout << signature_b64 << std::endl;

ASSERT_TRUE(signature::verify(
    signature_b64,
    nonce,
    base64::encode(keyPair.publicKey))
  );
}

TEST(Signature, PrintkeyPair){
  signature::KeyPair keyPair = signature::generateKeyPair();
  std::cout << base64::encode(keyPair.publicKey) << std::endl;
  std::cout << base64::encode(keyPair.privateKey) << std::endl;
}

TEST(Signature, generatedByAndroid){

 std::string private_key_b64 = "+BTZfRSPRgDdxmjZlK+QhJ3RQryMH23LIPqg5C/Eu2QcBoj3QM6ovTcmPok0iFYI1y9M683ZS4Ifp10jr9dQrQ==";
 std::string public_key_b64  = "b+etgin9x1S16omALSjr4HTVzv9IEXQzlvSTp7el0Js=";
 std::string signature_b64   = "HlJIjuds2OaSeyOjWjpnpXis55NvH3TD1SNVEwedu7sAY+Ypkksg3ovHUGfBhwd8uVmIX+JgnjrhKgPdyeO7DA==";
 std::string message = "0f1a39c82593e8b48e69f000c765c8e8072269d3bd4010634fa51d4e685076e30db22a9fb75def7379be0e808392922cb8c43d5dd5d5039828ed7ade7e1c6c81";

  ASSERT_TRUE(signature::verify(
    signature_b64,
    message,
    public_key_b64
  ));
}


TEST(Signature, generatedByiOS){

std::string public_key_b64  = "slyr7oz2+EU6dh2dY9+jNeO/hVrXCkT3rGhcNZo5rrE=";
std::string signature_b64   = "gdMUgjyo++4QpF1xDJNdk1a5zmDAEPM67WD4cn6CVZqDxC8nShb/L1Tokgo53HSOPDB0qXAVzcBvfcJ1WLjrAQ==";
std::string message = "46ed8c250356759f68930a94996faaa8f8c98ecbe0dcc58c479c8fad71e30096";

ASSERT_TRUE(signature::verify(
        signature_b64,
        message,
        public_key_b64
));
}

TEST(Signature, generatedByGO){

std::string public_key_b64  = "Oe8Xd5iDvVrVNc1apzLcdjRRq3ZZsTjkjebpDYdRcRw";
std::string signature_b64   = "FtLGaJLDK4g/tRzufBexe6fTAhjENtl6MWAynRpR9c1CZdEWJbbDS9svpU96hXiyGy3BQcwRxUz6eovBJdf6DQ==";
std::string message = "0f1a39c82593e8b48e69f000c765c8e8072269d3bd4010634fa51d4e685076e30db22a9fb75def7379be0e808392922cb8c43d5dd5d5039828ed7ade7e1c6c81";

ASSERT_TRUE(signature::verify(
        signature_b64,
        message,
        public_key_b64
));
}

