// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <axmol/math/Vec2.h>

namespace bim::axmol::app
{
  struct arena_display_config
  {
    ax::Vec2 grid_position_to_displayed_block_center(float x, float y) const;
    ax::Vec2 grid_position_to_display(float x, float y) const;

    /// Width, and height, of a displayed block in the arena view.
    float block_size;

    /// The height of the node containing the arena.
    float view_height;
  };
}
