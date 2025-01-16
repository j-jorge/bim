// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <chrono>

namespace bim::game
{
  entt::entity arena_reduction_factory(entt::registry& registry,
                                       std::chrono::milliseconds delay);
}
