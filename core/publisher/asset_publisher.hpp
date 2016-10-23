#ifndef __CORE_PUBLISHER_HPP__
#define __CORE_PUBLISHER_HPP__

#include <memory>

// What's a publisher? 
// A publisher is like factory in Domain-Driven Development.
namespace publisher{

    template<typename T, typename... Args>
    T&& publish(Args... args){
        T t(args...);
        return std::move(t);
    }

};

#endif // __CORE_PUBLISHER_HPP__