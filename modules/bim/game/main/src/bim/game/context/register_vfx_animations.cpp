// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/context/register_vfx_animations.hpp>

#include <bim/game/animation/animation_catalog.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/smoke_animations.hpp>

#include <entt/entity/registry.hpp>

void bim::game::register_vfx_animations(context& context,
                                        animation_catalog& animations)
{
  smoke_animations& smoke = context.create<smoke_animations>();

  smoke.in_out = animations.new_animation();

  {
    animation_specifications p;
    p.duration = std::chrono::milliseconds(330);
    p.dispatch_completion = [](entt::registry& r, entt::entity e) -> void
    {
      r.emplace<bim::game::dead>(e);
    };
    animations.replace_animation(smoke.in_out, p);
  }
}
