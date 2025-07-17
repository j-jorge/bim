// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;
  class context;

  void dump_arena(const arena& arena, const context& context,
                  const entt::registry& registry);
}
