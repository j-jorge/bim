// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/context/register_flame_animations.hpp>

#include <bim/game/animation/animation_catalog.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/constant/flame_duration.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/flame_animations.hpp>

#include <bim/assume.hpp>

#include <entt/entity/registry.hpp>

void bim::game::register_flame_animations(context& context,
                                          animation_catalog& animations)
{
  flame_animations& result = context.create<flame_animations>();

  result.warm_up = animations.new_animation();
  result.burn = animations.new_animation();
  result.cool_down = animations.new_animation();

  const std::chrono::milliseconds warm_up_duration(30 * 3);
  const std::chrono::milliseconds cool_down_duration(30 * 3);
  bim_assume(warm_up_duration + cool_down_duration < g_flame_duration);

  {
    animation_specifications p;
    p.duration = warm_up_duration;
    p.next = result.burn;
    animations.replace_animation(result.warm_up, p);
  }

  {
    animation_specifications p;
    p.duration = g_flame_duration - warm_up_duration - cool_down_duration;
    p.next = result.cool_down;
    animations.replace_animation(result.burn, p);
  }

  {
    animation_specifications p;
    p.duration = cool_down_duration;
    p.dispatch_completion = [](entt::registry& r, entt::entity e) -> void
    {
      r.emplace<dead>(e);
    };
    animations.replace_animation(result.cool_down, p);
  }
}
