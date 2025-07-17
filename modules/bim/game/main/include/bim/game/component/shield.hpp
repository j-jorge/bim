// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  struct shield
  {};

  bool has_shield(const entt::registry& registry, entt::entity e);
}
