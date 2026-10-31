// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#ifndef NDEBUG
  #include <bim/assert.hpp>
  #define bim_assume(b) bim_assert(b)
#elif defined(__GNUC__)
  #define bim_assume(b)                                                       \
    do                                                                        \
      {                                                                       \
        if (!(b))                                                             \
          __builtin_unreachable();                                            \
      }                                                                       \
    while (false)
#else
  #define bim_assume(b)
#endif
