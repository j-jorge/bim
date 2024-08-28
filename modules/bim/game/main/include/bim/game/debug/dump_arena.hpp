// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;

  void dump_arena(const arena& arena, const entt::registry& registry);
}
