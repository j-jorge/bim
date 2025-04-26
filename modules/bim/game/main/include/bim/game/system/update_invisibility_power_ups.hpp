// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;

  void update_invisibility_power_ups(entt::registry& registry, arena& arena);
}
