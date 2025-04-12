// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <chrono>

namespace bim::game
{
  class arena;
  class context;

  void update_falling_blocks(const context& context, entt::registry& registry,
                             arena& arena);
}
