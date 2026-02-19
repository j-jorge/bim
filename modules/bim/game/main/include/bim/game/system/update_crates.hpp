// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class context;

  void update_crates(const context& context, entt::registry& registry);
}
