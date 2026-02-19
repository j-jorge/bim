// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/context/fill_context.hpp>

#include <bim/game/animation/animation_catalog.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/register_crate_animations.hpp>
#include <bim/game/context/register_flame_animations.hpp>
#include <bim/game/context/register_player_animations.hpp>
#include <bim/game/context/register_vfx_animations.hpp>

void bim::game::fill_context(context& context)
{
  animation_catalog& animations = context.create<animation_catalog>();

  register_crate_animations(context, animations);
  register_flame_animations(context, animations);
  register_player_animations(context, animations);
  register_vfx_animations(context, animations);
}
