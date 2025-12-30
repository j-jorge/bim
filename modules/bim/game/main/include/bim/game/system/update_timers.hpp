// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <chrono>

namespace bim::game
{
  void update_timers(entt::registry& registry,
                     std::chrono::milliseconds elapsed_time);
  void trigger_crushed_timers(entt::registry& registry);
}
