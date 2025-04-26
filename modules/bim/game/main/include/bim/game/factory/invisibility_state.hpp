// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <chrono>

namespace bim::game
{
  void invisibility_state_factory(entt::registry& registry, entt::entity e,
                                  std::chrono::milliseconds duration);
}
