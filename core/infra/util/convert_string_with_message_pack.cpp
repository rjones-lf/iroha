
#include "../../util/convert_string.hpp"
#include "../../model/transactions/abstract_transaction.hpp"

namespace convert {

    using Abs_tx = abstract_transaction::AbstractTransaction;

    template<typename T>
    std::string to_string(std::unique_ptr<T> object){
        std::stringstream buffer;
        msgpack::pack(buffer, *object);
        buffer.seekg(0);
        return std::string(buffer.str());
    }

    template<typename T>
    T to_object(std::string msg){
        msgpack::object_handle oh =
        msgpack::unpack(msg.data(), msg.size());

        msgpack::object deserialized = oh.get();
        T dst;
        deserialized.convert(dst);
        return dst;
    }
};
