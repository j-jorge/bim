// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>
#include <vector>

namespace ax
{
  class Sprite;
  class SpriteFrame;
}

namespace bim::game
{
  struct animation_state;
}

namespace bim::axmol::widget
{
  struct animation
  {
    struct frame
    {
      ax::SpriteFrame* sprite_frame;
      std::chrono::milliseconds display_date;
      float angle;
      bool flip_x;
    };

    std::vector<frame> frames;
    std::chrono::milliseconds total_duration;

    void apply(ax::Sprite& sprite,
               const bim::game::animation_state& state) const;
  };
}
