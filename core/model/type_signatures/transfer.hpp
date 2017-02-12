#ifndef CORE_MODEL_TYPE_SIGNATURES_TRANSFER_HPP
#define CORE_MODEL_TYPE_SIGNATURES_TRANSFER_HPP

#include "tags.hpp"

namespace type_signatures {

template <class... Ts>
struct Transfer {
  using type = typename detail::head< Ts... >::type;
  object::Object object;
  Transfer(type o): object(o) {}
};

}



#endif