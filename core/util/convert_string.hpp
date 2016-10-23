#ifndef __CORE_REPOSITORY_CONVERT_STRING_HPP__
#define __CORE_REPOSITORY_CONVERT_STRING_HPP__

#include <string>
#include <sstream>

namespace convert{
    
    template<typename T>
    std::string to_string(std::unique_ptr<T> object);

    template<typename T>
    T to_object(std::string msg);

};

#endif