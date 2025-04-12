// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/contest_result.hpp>

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class contest_result;
  class context;

  contest_result check_game_over(const context& context,
                                 const entt::registry& registry);
}
