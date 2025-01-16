// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <chrono>

namespace bim::game
{
  struct position_on_grid;

  entt::entity falling_block_factory(entt::registry& registry,
                                     const position_on_grid& position,
                                     std::chrono::milliseconds fall_duration);
}
