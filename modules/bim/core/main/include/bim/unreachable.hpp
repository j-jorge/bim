// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#ifndef NDEBUG
  #include <cassert>
  #define bim_unreachable assert(false)
  #define bim_unreachable_in_release                                          \
    do                                                                        \
      {                                                                       \
      }                                                                       \
    while (0)
#else
  #define bim_unreachable __builtin_unreachable()
  #define bim_unreachable_in_release __builtin_unreachable()
#endif
