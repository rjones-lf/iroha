

#include "../../model/smart_contract/virtual_machine_interface.hpp"
#include "jvm/java_virtual_machine.hpp"
#include "../../util/logger.hpp"
#include <map>

namespace smart_contract {

    std::map<std::string, std::unique_ptr<JavaContext>> vmSet;

    void SmartContract::initializeVM(const std::string& contractName){
         vmSet.insert(std::make_pair( contractName, smart_contract::initializeVM(contractName)));
    }

    void SmartContract::finishVM(const std::string& contractName){
         if(vmSet.find(contractName) != vmSet.end()){
            vmSet.erase(contractName);
         }
    }

    void SmartContract::invokeFunction(
        const std::string& contractName,
        const std::string& functionName,
        const std::unordered_map<std::string, std::string>& params
    ) {
        if(vmSet.find(contractName) != vmSet.end()){
            const auto& context = vmSet.at(contractName);
            smart_contract::execFunction( context, functionName, params);
        }
    }
};


