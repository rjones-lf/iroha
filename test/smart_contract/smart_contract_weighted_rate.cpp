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

#include <../smart_contract/repository/jni_constants.hpp>
#include <infra/protobuf/api.pb.h>
#include <infra/virtual_machine/jvm/java_data_structure.hpp>
#include <repository/domain/account_repository.hpp>
#include <repository/world_state_repository.hpp>
#include <virtual_machine/virtual_machine.hpp>

const std::string PackageName = "sample_rating";
const std::string ContractName = "WeightedRate";

namespace tag = jni_constants;

/*
 * This test is not in CMakeLists.txt because currently, error code
 * from Java exec cannot be caught.
 */

TEST(SmartContractSample, weightedRate
) {
virtual_machine::initializeVM(PackageName, ContractName
);
virtual_machine::invokeFunction(PackageName, ContractName,
"testMain");
virtual_machine::finishVM(PackageName, ContractName
);
}
