/*
 * gpu_check.hpp - Minimal macro front-end for Vulkan result checking
 *
 */

#pragma once

#include <cstdint>
#include <expected>
#include <source_location>
#include <string>
#include <vulkan/vulkan.hpp>

#include "pandora/core/err/dispatch.hpp"
#include "pandora/core/err/error.hpp"
#include "pandora/core/err/result.hpp"

namespace pandora::core::gpu {

struct error_policy {
  static constexpr bool use_exceptions =
#if defined(PLB_NO_EXCEPTIONS)
      false;
#else
      true;
#endif
};

err::Error map_vk_result(vk::Result r, const char* expr_text);

inline err::Status check_vk(
    vk::Result r,
    const char* expr_text,
    const char* api_name = nullptr,
    const char* detail = nullptr,
    std::uint64_t context_id = 0,
    std::source_location loc = std::source_location::current()) {
  if (r == vk::Result::eSuccess) {
    return {};
  }
  auto e = map_vk_result(r, expr_text);
  // Augment message lazily
  if (api_name)
    e.message += std::string(" api=") + api_name;
  if (detail)
    e.message += std::string(" detail=") + detail;
  e.context_id = context_id;
  e.where = loc;
  // First dispatch for logging/metrics before propagating.
  ::pandora::core::err::dispatch_error(e);
  if constexpr (error_policy::use_exceptions) {
    err::throw_error(e);  // noreturn
    return {};            // placate compiler
  } else {
    return std::unexpected(e);
  }
}

}  // namespace pandora::core::gpu

// end inline helpers

// 最頻出マクロ: 呼び出し点文字列化 + check_vk ディスパッチ
#if !defined(PLB_GPU_CHECK)
  #define PLB_GPU_CHECK(expr)                                                \
    do {                                                                     \
      auto _plb_res = (expr);                                                \
      auto _plb_status = ::pandora::core::gpu::check_vk(_plb_res, #expr);    \
      if (!_plb_status) {                                                    \
        if constexpr (!::pandora::core::gpu::error_policy::use_exceptions) { \
          return std::unexpected(_plb_status.error());                       \
        }                                                                    \
      }                                                                      \
    } while (0)
#endif
