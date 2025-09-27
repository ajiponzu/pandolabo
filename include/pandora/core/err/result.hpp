#pragma once
#include <expected>

#include "pandora/core/err/error.hpp"

namespace pandora::core::err {

template <class T>
using Result = std::expected<T, Error>;
using Status = std::expected<void, Error>;  // monostate not needed

}  // namespace pandora::core::err

// Helper macros for early return. Intentionally small & side-effect safe.
// Usage:
//   PLB_TRY(do_something());                // for Status
//   PLB_TRY_ASSIGN(val, make_value());      // for Result<T>
// They propagate either by throwing (when exceptions enabled) or by returning
// unexpected.

#if !defined(PLB_TRY)
  #define PLB_TRY(expr)                                  \
    do {                                                 \
      auto _plb_status_tmp = (expr);                     \
      if (!_plb_status_tmp) {                            \
        return std::unexpected(_plb_status_tmp.error()); \
      }                                                  \
    } while (0)
#endif

#if !defined(PLB_TRY_ASSIGN)
  #define PLB_TRY_ASSIGN(lhs, expr)                      \
    do {                                                 \
      auto _plb_result_tmp = (expr);                     \
      if (!_plb_result_tmp) {                            \
        return std::unexpected(_plb_result_tmp.error()); \
      }                                                  \
      (lhs) = std::move(_plb_result_tmp.value());        \
    } while (0)
#endif
