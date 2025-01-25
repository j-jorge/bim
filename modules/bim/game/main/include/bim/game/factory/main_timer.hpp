// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <chrono>

namespace bim::game
{
  entt::entity main_timer_factory(entt::registry& registry,
                                  std::chrono::milliseconds fall_duration);
}
