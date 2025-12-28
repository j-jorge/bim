// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/game_feature_sprite_frame.hpp>

#include <bim/game/feature_flags.hpp>

#include <axmol/2d/SpriteFrameCache.h>

ax::SpriteFrame&
bim::axmol::app::game_feature_sprite_frame(bim::game::feature_flags f)
{
  const char* sprite_frame_name = nullptr;

  switch (f)
    {
    case bim::game::feature_flags::falling_blocks:
      sprite_frame_name = "sprites/features/falling-blocks.png";
      break;
    case bim::game::feature_flags::fences:
      sprite_frame_name = "sprites/features/fences.png";
      break;
    case bim::game::feature_flags::invisibility:
      sprite_frame_name = "sprites/features/invisibility.png";
      break;
    case bim::game::feature_flags::shield:
      sprite_frame_name = "sprites/features/shield.png";
      break;
    case bim::game::feature_flags::fog_of_war:
      sprite_frame_name = "sprites/features/fog-of-war.png";
      break;
    }

  assert(sprite_frame_name != nullptr);
  return *ax::SpriteFrameCache::getInstance()->findFrame(sprite_frame_name);
}
