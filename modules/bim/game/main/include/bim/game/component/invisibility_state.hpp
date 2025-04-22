// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  struct invisibility_state
  {
    entt::entity entity;
  };

  bool is_invisible(const entt::registry& registry, entt::entity entity);
}
