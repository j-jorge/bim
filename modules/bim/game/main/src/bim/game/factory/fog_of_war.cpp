// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/fog_of_war.hpp>

#include <bim/game/cell_neighborhood.hpp>
#include <bim/game/component/fog_of_war.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/constant/fog_roll_in_duration.hpp>

#include <bim/table_2d.impl.hpp>

#include <entt/entity/registry.hpp>

void bim::game::fog_of_war_factory(
    entt::registry& registry, std::uint8_t player_index, int arena_width,
    int arena_height, const std::span<const position_on_grid>& exclude)
{
  int player_x;
  int player_y;
  bool player_found = false;

  for (const auto& [entity, player, position] :
       registry.view<player, fractional_position_on_grid>().each())
    if (player.index == player_index)
      {
        player_x = position.grid_aligned_x();
        player_y = position.grid_aligned_y();
        player_found = true;
        break;
      }

  if (!player_found)
    return;

  // Build a map of the cells containing fog. We add a margin of one cell all
  // around to simplify the neighborhood construction that comes after.
  table_2d<bool> fog_map(arena_width + 2, arena_height + 2, true);

  // No fog on the top and bottom borders.
  for (int x = 0; x < (int)fog_map.width(); ++x)
    {
      fog_map(x, 0) = false;
      fog_map(x, arena_height + 1) = false;
    }

  // No fog on the left and right borders.
  for (int y = 0; y < (int)fog_map.height(); ++y)
    {
      fog_map(0, y) = false;
      fog_map(arena_width + 1, y) = false;
    }

  // No fog around the player.
  for (int y = std::max(player_y - 1, 0);
       y <= std::min(player_y + 1, arena_height - 1); ++y)
    for (int x = std::max(player_x - 1, 0);
         x <= std::min(player_x + 1, arena_width - 1); ++x)
      fog_map(x + 1, y + 1) = false;

  // No fog in the cells from the excluded list.
  for (const position_on_grid& p : exclude)
    fog_map(p.x + 1, p.y + 1) = false;

  // Now we create the entities and sets the neighborhood according to the map.
  for (int y = 0; y < arena_height; ++y)
    for (int x = 0; x < arena_width; ++x)
      {
        const int mx = x + 1;
        const int my = y + 1;

        if (!fog_map(mx, my))
          continue;

        const entt::entity e = registry.create();
        cell_neighborhood neighborhood = cell_neighborhood::none;

        if (fog_map(mx - 1, my - 1))
          neighborhood |= cell_neighborhood::up_left;
        if (fog_map(mx, my - 1))
          neighborhood |= cell_neighborhood::up;
        if (fog_map(mx + 1, my - 1))
          neighborhood |= cell_neighborhood::up_right;

        if (fog_map(mx - 1, my))
          neighborhood |= cell_neighborhood::left;
        if (fog_map(mx + 1, my))
          neighborhood |= cell_neighborhood::right;

        if (fog_map(mx - 1, my + 1))
          neighborhood |= cell_neighborhood::down_left;
        if (fog_map(mx, my + 1))
          neighborhood |= cell_neighborhood::down;
        if (fog_map(mx + 1, my + 1))
          neighborhood |= cell_neighborhood::down_right;

        constexpr int opacity = 0;
        registry.emplace<fog_of_war>(e, player_index, opacity, neighborhood,
                                     fog_state::roll_in);
        registry.emplace<position_on_grid>(e, x, y);

        const std::chrono::milliseconds roll_in_delay(
            (std::abs(x - arena_width / 2) + std::abs(y - arena_height / 2))
            * 15);
        registry.emplace<timer>(e, g_fog_roll_in_duration + roll_in_delay);
      }
}
