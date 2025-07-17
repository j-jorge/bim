// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_shields.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/shield.hpp>
#include <bim/game/factory/invincibility_state.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_shields(entt::registry& registry)
{
  registry.view<burning, shield>().each(
      [&](entt::entity e) -> void
      {
        registry.erase<burning>(e);
        registry.erase<shield>(e);

        invincibility_state_factory(registry, e, std::chrono::seconds(5));
      });
}
