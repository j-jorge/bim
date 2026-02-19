// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_crates.hpp>

#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/crate.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/crate_animations.hpp>
#include <bim/game/factory/smoke_vfx.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_crates(const context& context, entt::registry& registry)
{
  const crate_animations& animations = context.get<const crate_animations>();

  registry.view<crate, burning, position_on_grid, animation_state>().each(
      [&](entt::entity e, position_on_grid position,
          animation_state& state) -> void
      {
        if (state.model != animations.idle)
          return;

        state.transition_to(animations.burn);

        // Smoke with a short delay.
        in_out_smoke_vfx_factory(context, registry, position.x, position.y,
                                 std::chrono::milliseconds(-4 * 45));
      });
}
