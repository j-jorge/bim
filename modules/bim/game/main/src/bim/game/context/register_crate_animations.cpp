// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/context/register_crate_animations.hpp>

#include <bim/game/animation/animation_catalog.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/crate_animations.hpp>

#include <entt/entity/registry.hpp>

void bim::game::register_crate_animations(context& context,
                                          animation_catalog& animations)
{
  crate_animations& result = context.create<crate_animations>();

  result.idle = animations.new_animation();
  result.burn = animations.new_animation();

  {
    animation_specifications p;
    p.duration = std::chrono::milliseconds(270);
    p.dispatch_completion = [](entt::registry& r, entt::entity e) -> void
    {
      r.emplace<dead>(e);
    };
    animations.replace_animation(result.burn, p);
  }
}
