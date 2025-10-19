// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/feature_flags_fwd.hpp>

namespace ax
{
  class SpriteFrame;
}

namespace bim::axmol::app
{
  ax::SpriteFrame& game_feature_sprite_frame(bim::game::feature_flags f);
}
