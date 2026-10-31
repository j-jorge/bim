// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#ifndef NDEBUG
  #include <cstdlib>
  #include <iscool/log/log.hpp>
  #include <iscool/log/nature/error.hpp>

  #define bim_assert(b)                                                       \
    do                                                                        \
      {                                                                       \
        if (!(b))                                                             \
          {                                                                   \
            try                                                               \
              {                                                               \
                ic_log(iscool::log::nature::error(), "assert_failed", "{}",   \
                       #b);                                                   \
              }                                                               \
            catch (const std::format_error&)                                  \
              {}                                                              \
            std::abort();                                                     \
          }                                                                   \
      }                                                                       \
    while (false)
#else
  #define bim_assert(b)                                                       \
    do                                                                        \
      {                                                                       \
      }                                                                       \
    while (false)
#endif
