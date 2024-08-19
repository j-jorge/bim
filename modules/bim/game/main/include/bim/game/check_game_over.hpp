// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/contest_result.hpp>

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  struct contest_result;

  contest_result check_game_over(const entt::registry& registry);
}
