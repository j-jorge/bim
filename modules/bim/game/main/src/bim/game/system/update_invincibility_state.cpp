// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_invincibility_state.hpp>

#include <bim/game/component/burning.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/invincibility_state.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_invincibility_state(entt::registry& registry)
{
  registry.view<invincibility_state>().each(
      [&](entt::entity e, invincibility_state& s) -> void
      {
        const timer& t = registry.get<timer>(s.entity);

        if (t.duration.count() == 0)
          {
            registry.emplace<dead>(s.entity);
            registry.erase<invincibility_state>(e);
          }
        else
          registry.remove<burning>(e);
      });
}
