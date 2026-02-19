// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/animation/animation.hpp>

#include <bim/game/component/animation_state.hpp>

#include <bim/assume.hpp>

#include <axmol/2d/Sprite.h>

void bim::axmol::widget::animation::apply(
    ax::Sprite& sprite, const bim::game::animation_state& state) const
{
  bim_assume(frames.size() > 0);

  std::chrono::milliseconds t = state.elapsed_time;

  if (t.count() < 0)
    {
      sprite.setOpacity(0);
      return;
    }

  sprite.setOpacity(255);

  if (total_duration.count() > 0)
    t = t % total_duration;

  const frame* frame = &frames.back();

  for (std::size_t i = 1, n = frames.size(); i != n; ++i)
    if (frames[i].display_date > t)
      {
        frame = &frames[i - 1];
        break;
      }

  sprite.setSpriteFrame(frame->sprite_frame);
  sprite.setFlippedX(frame->flip_x);
  sprite.setRotation(frame->angle);
}
