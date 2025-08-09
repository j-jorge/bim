// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/remove_dead_objects.hpp>

#include <bim/game/component/dead.hpp>
#include <bim/game/component/invincibility_state.hpp>
#include <bim/game/component/invisibility_state.hpp>

#include <entt/entity/registry.hpp>

void bim::game::remove_dead_objects(entt::registry& registry)
{
  registry.view<dead, invincibility_state>().each(
      [&](entt::entity e, const invincibility_state& s) -> void
      {
        registry.destroy(s.entity);
        registry.destroy(e);
      });

  registry.view<dead, invisibility_state>().each(
      [&](entt::entity e, const invisibility_state& s) -> void
      {
        registry.destroy(s.entity);
        registry.destroy(e);
      });

  registry.view<dead>().each(
      [&](entt::entity e) -> void
      {
        registry.destroy(e);
      });
}
