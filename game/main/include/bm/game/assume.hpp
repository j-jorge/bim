#pragma once

#include <cassert>

#ifndef NDEBUG
  #define bm_assume(b) assert(b)
#elif defined(__GNUC__)
  #define bm_assume(b)                                                        \
    do                                                                        \
      {                                                                       \
        if(b)                                                                 \
          __builtin_unreachable();                                            \
      }                                                                       \
    while(false)
#else
  #define bm_assume(b)
#endif
