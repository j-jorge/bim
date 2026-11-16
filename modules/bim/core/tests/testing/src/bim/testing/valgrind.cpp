// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/testing/valgrind.hpp>

#include <cstdlib>
#include <string_view>

bool bim::testing::is_running_in_valgrind()
{
  const char* const e = std::getenv("LD_PRELOAD");

  return e && std::string_view(e).contains("/valgrind/");
}
