// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/arena_display_config.hpp>

ax::Vec2
bim::axmol::app::arena_display_config::grid_position_to_displayed_block_center(
    float x, float y) const
{
  const float center_x = (float)x * block_size + block_size / 2;
  const float center_y = view_height - block_size / 2 - (float)y * block_size;

  return ax::Vec2(center_x, center_y);
}

ax::Vec2
bim::axmol::app::arena_display_config::grid_position_to_display(float x,
                                                                float y) const
{
  const float view_x = x * block_size;
  const float view_y = view_height - y * block_size;

  return ax::Vec2(view_x, view_y);
}
