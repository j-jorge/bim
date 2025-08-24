// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstddef>

namespace bim::server::tests
{
  struct statistics_log_line
  {
    std::size_t active_sessions;
    std::size_t players_in_games;
    std::size_t games;
  };
}
