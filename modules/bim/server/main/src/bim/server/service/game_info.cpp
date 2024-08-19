// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/game_info.hpp>

#include <algorithm>

std::size_t
bim::server::game_info::session_index(iscool::net::session_id session) const
{
  return std::find(sessions.begin(), sessions.end(), session)
         - sessions.begin();
}
