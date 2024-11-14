// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/dump_arena.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/constant/max_player_count.hpp>

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

  for (int y = 0; y != h; ++y)
    for (int x = 0; x != w; ++x)
      {
        if (arena.entity_at(x, y) != entt::null)
          arena_str[y][x] = "!";

        if (arena.is_solid(x, y))
          arena_str[y][x] = "░";

        if (arena.is_static_wall(x, y))
          arena_str[y][x] = "▒";
      }

  std::vector<player> players;
  std::array<entt::entity, g_max_player_count> player_entities;
  std::array<fractional_position_on_grid, g_max_player_count> player_position;

  registry.view<player, fractional_position_on_grid>().each(
      [&](entt::entity e, const player& p,
          fractional_position_on_grid pos) -> void
      {
        arena_str[pos.grid_aligned_y()][pos.grid_aligned_x()] =
            "ABCD"[p.index];
        if (players.size() <= p.index)
          players.resize(p.index + 1);

        players[p.index] = p;
        player_entities[p.index] = e;
        player_position[p.index] = pos;
      });

  const int player_count = players.size();

  registry.view<flame, position_on_grid>().each(
      [&](const flame& f, position_on_grid pos) -> void
      {
        switch (f.segment)
          {
          case flame_segment::origin:
            arena_str[pos.y][pos.x] = "+";
            break;
          case flame_segment::arm:
            switch (f.direction)
              {
              case flame_direction::right:
              case flame_direction::left:
                arena_str[pos.y][pos.x] = "-";
                break;
              case flame_direction::up:
              case flame_direction::down:
                arena_str[pos.y][pos.x] = "|";
                break;
              }
            break;
          case flame_segment::tip:
            switch (f.direction)
              {
              case flame_direction::right:
                arena_str[pos.y][pos.x] = ">";
                break;
              case flame_direction::left:
                arena_str[pos.y][pos.x] = "<";
                break;
              case flame_direction::up:
                arena_str[pos.y][pos.x] = "^";
                break;
              case flame_direction::down:
                arena_str[pos.y][pos.x] = "v";
                break;
              }
            break;
          }
      });

  registry.view<flame_power_up, position_on_grid>().each(
      [&](position_on_grid pos) -> void
      {
        arena_str[pos.y][pos.x] = "F";
      });

  struct bomb_state
  {
    std::uint8_t strength;
    std::uint8_t player;
    std::uint8_t x;
    std::uint8_t y;
  };

  std::vector<bomb_state> bombs;

  registry.view<bomb, position_on_grid>().each(
      [&](const bomb& b, position_on_grid pos) -> void
      {
        arena_str[pos.y][pos.x] = "ó";
        bombs.emplace_back(b.strength, b.player_index, pos.x, pos.y);
      });

  std::sort(bombs.begin(), bombs.end(),
            [](bomb_state lhs, bomb_state rhs) -> bool
            {
              return (lhs.y < rhs.y) || ((lhs.y == rhs.y) && (lhs.x < rhs.x));
            });

  registry.view<bomb_power_up, position_on_grid>().each(
      [&](position_on_grid pos) -> void
      {
        arena_str[pos.y][pos.x] = "Ó";
      });

  int arena_print_y = 0;
  const auto print_arena_line = [&arena_str, &arena_print_y, w,
                                 h](char eol) -> void
  {
    if (arena_print_y >= h)
      return;

    printf("%x", arena_print_y);
    for (int x = 0; x != w; ++x)
      printf("%s", arena_str[arena_print_y][x].c_str());
    printf("%c", eol);

    ++arena_print_y;
  };

  printf(" ");
  for (int i = 0; i != w; ++i)
    printf("%x", i);

  printf(" %-15s", "player_index");
  for (const player& p : players)
    printf("%9d", (int)p.index);
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "player_entity");
  for (int i = 0; i != player_count; ++i)
    printf("%9x", (int)player_entities[i]);
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "bomb_capacity");
  for (const player& p : players)
    printf("%9d", (int)p.bomb_capacity);
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "bomb_available");
  for (const player& p : players)
    printf("%9d", (int)p.bomb_available);
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "bomb_strength");
  for (const player& p : players)
    printf("%9d", (int)p.bomb_strength);
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "x");
  for (int i = 0; i != player_count; ++i)
    printf("%9.4f", player_position[i].x_float());
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "y");
  for (int i = 0; i != player_count; ++i)
    printf("%9.4f", player_position[i].y_float());
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "x_grid");
  for (int i = 0; i != player_count; ++i)
    printf("%9d", (int)player_position[i].grid_aligned_x());
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "y_grid");
  for (int i = 0; i != player_count; ++i)
    printf("%9d", (int)player_position[i].grid_aligned_y());
  printf("\n");

  std::array<player_action, g_max_player_count> player_actions;

  registry.view<player, player_action>().each(
      [&player_actions](const player& p, const player_action& action)
      {
        player_actions[p.index] = action;
      });

  print_arena_line(' ');
  printf("%-15s", "action");
  for (int i = 0; i != player_count; ++i)
    {
      char d = '.';

      switch (player_actions[i].movement)
        {
        case player_movement::idle:
          d = '.';
          break;
        case player_movement::up:
          d = '^';
          break;
        case player_movement::down:
          d = 'v';
          break;
        case player_movement::left:
          d = '<';
          break;
        case player_movement::right:
          d = '>';
          break;
        };

      printf("       %c%c", d, player_actions[i].drop_bomb ? 'b' : '.');
    }
  printf("\n");

  std::size_t bomb_print_i = 0;
  const auto print_bombs_line = [&bombs, &bomb_print_i]() -> void
  {
    for (int i = 0; i != 4; ++i)
      {
        if (bomb_print_i >= bombs.size())
          break;

        const bomb_state b = bombs[bomb_print_i];
        ++bomb_print_i;

        if (i != 0)
          printf(" ");

        printf("(%d, %d, %d, %d)", (int)b.x, (int)b.y, (int)b.player,
               (int)b.strength);
      }

    printf("\n");
  };

  if (!bombs.empty())
    {
      print_arena_line(' ');
      printf("bombs(x, y, player, strength):\n");

      while ((arena_print_y != h) && (bomb_print_i != bombs.size()))
        {
          print_arena_line(' ');
          print_bombs_line();
        }
    }

  while (arena_print_y != h)
    print_arena_line('\n');

  while (bomb_print_i != bombs.size())
    print_bombs_line();
}
