// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/context/register_player_animations.hpp>

#include <bim/game/animation/animation_catalog.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>

#include <entt/entity/registry.hpp>

void bim::game::register_player_animations(context& context,
                                           animation_catalog& animations)
{
  player_animations& result = context.create<player_animations>();

  result.idle_down = animations.new_animation();
  result.idle_left = animations.new_animation();
  result.idle_right = animations.new_animation();
  result.idle_up = animations.new_animation();

  result.walk_down = animations.new_animation();
  result.walk_left = animations.new_animation();
  result.walk_right = animations.new_animation();
  result.walk_up = animations.new_animation();

  result.burn = animations.new_animation();
  result.die = animations.new_animation();

  {
    animation_specifications p;
    p.duration = std::chrono::seconds(1);
    p.dispatch_completion = [](entt::registry& r, entt::entity e) -> void
    {
      r.emplace<bim::game::dead>(e);
    };
    animations.replace_animation(result.burn, p);
  }

  {
    animation_specifications p;
    p.duration = std::chrono::milliseconds(240);
    p.dispatch_completion = [](entt::registry& r, entt::entity e) -> void
    {
      r.emplace<bim::game::dead>(e);
    };
    animations.replace_animation(result.die, p);
  }
}
