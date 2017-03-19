
#ifdef __has_include
#  if __has_include(<optional>)
#    include <optional>
#    define have_optional 1
#  elif __has_include(<experimental/optional>)
#    include <experimental/optional>
#    define have_optional 1
#    define experimental_optional
#  else
#    define have_optional 0
#    error "This file requires to use std::experimental::optional (or std::optional). Please update GCC version."
#  endif
#endif

#ifdef experimental_optional
using std::experimental::optional;
using std::experimental::make_optional;
using std::experimental::nullopt;
#else
using std::optional
using std::make_optional;
using std:::
nullopt;
#endif