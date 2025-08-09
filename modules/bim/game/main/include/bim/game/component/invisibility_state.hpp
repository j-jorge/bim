// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  /**
   * The invisibility_state is attached to the entity we want to be
   * invisible. The entity **owned** by the state is used to manage the state's
   * state. This will receive the timer.
   */
  struct invisibility_state
  {
    entt::entity entity;
  };

  bool is_invisible(const entt::registry& registry, entt::entity entity);
}
