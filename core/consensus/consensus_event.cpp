#include "consensus_event.hpp"

namespace event {

    template<typename T>
    using Transaction = transaction::Transaction<T>;
    template<typename T>
    using Transfer = command::Transfer<T>;
    template<typename T>
    using Add = command::Add<T>;

    template<typename T>
    using Update = command::Update<T>;
};
