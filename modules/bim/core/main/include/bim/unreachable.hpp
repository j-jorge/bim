// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#ifdef NDEBUG
  #define bim_unreachable_in_release __builtin_unreachable()
#else
  #define bim_unreachable_in_release                                          \
    do                                                                        \
      {                                                                       \
      }                                                                       \
    while (0)
#endif
