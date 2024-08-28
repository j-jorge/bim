// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/debug/dump_arena.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

#include <cstdio>
#include <string>
#include <vector>

void bim::game::dump_arena(const arena& arena, const entt::registry& registry)
{
  const int w = arena.width();
  const int h = arena.height();

  std::vector<std::vector<std::string>> arena_str(
      arena.height(), std::vector<std::string>(arena.width(), " "));

  for (int y = 0; y != h; ++y)
    for (int x = 0; x != w; ++x)
      {
        if (arena.is_static_wall(x, y))
          arena_str[y][x] = "%";

        if (arena.is_solid(x, y))
          arena_str[y][x] = "#";

        if (arena.entity_at(x, y) != entt::null)
          arena_str[y][x] = "!";
      }

  registry.view<fractional_position_on_grid>().each(
      [&](fractional_position_on_grid pos) -> void
      {
        arena_str[pos.grid_aligned_y()][pos.grid_aligned_x()] = "?";
      });

  registry.view<position_on_grid>().each(
      [&](position_on_grid pos) -> void
      {
        arena_str[pos.y][pos.x] = "?";
      });

  std::vector<entt::entity> player_entities;
  std::vector<player> players;

  registry.view<player, fractional_position_on_grid>().each(
      [&](entt::entity e, const player& p,
          fractional_position_on_grid pos) -> void
      {
        arena_str[pos.grid_aligned_y()][pos.grid_aligned_x()] =
            std::to_string((int)p.index);
        player_entities.push_back(e);
        players.push_back(p);
      });

  registry.view<flame, position_on_grid>().each(
      [&](const flame&, position_on_grid pos) -> void
      {
        arena_str[pos.y][pos.x] = "f";
      });

  registry.view<flame_power_up, position_on_grid>().each(
      [&](position_on_grid pos) -> void
      {
        arena_str[pos.y][pos.x] = "F";
      });

  registry.view<bomb, position_on_grid>().each(
      [&](const bomb&, position_on_grid pos) -> void
      {
        arena_str[pos.y][pos.x] = "b";
      });

  registry.view<bomb_power_up, position_on_grid>().each(
      [&](position_on_grid pos) -> void
      {
        arena_str[pos.y][pos.x] = "B";
      });

  printf(" ");
  for (int i = 0; i != w; ++i)
    printf("%x", i);
  printf("\n");

  for (int y = 0; y != h; ++y)
    {
      printf("%x", y);
      for (int x = 0; x != w; ++x)
        printf("%s", arena_str[y][x].c_str());
      printf("\n");
    }

  printf("%-15s", "player_index");
  for (const player& p : players)
    printf("%9d", (int)p.index);
  printf("\n");

  printf("%-15s", "player_entity");
  for (const entt::entity e : player_entities)
    printf("%9x", (int)e);
  printf("\n");

  printf("%-15s", "bomb_capacity");
  for (const player& p : players)
    printf("%9d", (int)p.bomb_capacity);
  printf("\n");

  printf("%-15s", "bomb_available");
  for (const player& p : players)
    printf("%9d", (int)p.bomb_available);
  printf("\n");

  printf("%-15s", "bomb_strength");
  for (const player& p : players)
    printf("%9d", (int)p.bomb_strength);
  printf("\n");
}
