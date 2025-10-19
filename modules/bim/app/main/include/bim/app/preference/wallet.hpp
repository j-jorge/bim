// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::app
{
  std::int64_t coins_balance(const iscool::preferences::local_preferences& p);
  void coins_balance(iscool::preferences::local_preferences& p,
                     std::int64_t v);

  void add_coins(iscool::preferences::local_preferences& p, std::int64_t a);
  void consume_coins(iscool::preferences::local_preferences& p,
                     std::int64_t a);
}
