// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cassert>

#ifndef NDEBUG
  #define bim_assume(b) assert(b)
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
