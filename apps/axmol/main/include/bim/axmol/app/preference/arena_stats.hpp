// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::axmol::app
{
  std::int64_t games_in_arena(const iscool::preferences::local_preferences& p);
  void games_in_arena(iscool::preferences::local_preferences& p,
                      std::int64_t v);

  std::int64_t
  victories_in_arena(const iscool::preferences::local_preferences& p);
  void victories_in_arena(iscool::preferences::local_preferences& p,
                          std::int64_t v);

  std::int64_t
  defeats_in_arena(const iscool::preferences::local_preferences& p);
  void defeats_in_arena(iscool::preferences::local_preferences& p,
                        std::int64_t v);
}
