// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_invisibility_state.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_invisibility_state(const context& context,
                                          entt::registry& registry)
{
  const player_animations& animations = context.get<const player_animations>();

  registry.view<invisibility_state, player, animation_state>().each(
      [&](entt::entity e, invisibility_state& s, const player&,
          const animation_state& a) -> void
      {
        const timer& t = registry.get<timer>(s.entity);

        if ((t.duration.count() == 0) || !animations.is_alive(a.model))
          {
            registry.emplace<dead>(s.entity);
            registry.erase<invisibility_state>(e);
          }
      });
}
