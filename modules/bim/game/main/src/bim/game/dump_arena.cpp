// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/dump_arena.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/invincibility_state.hpp>
#include <bim/game/component/invisibility_power_up.hpp>
#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/component/kicked.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/shield.hpp>
#include <bim/game/component/shield_power_up.hpp>
#include <bim/game/component/solid.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

#include <cstdio>
#include <string>
#include <vector>

static void dump_column(bool valid, float value)
{
  if (valid)
    printf("%10.4f", value);
  else
    printf("         x");
}

static void dump_column(bool valid, int value)
{
  if (valid)
    printf("%10d", value);
  else
    printf("         x");
}

static void dump_column_hex(bool valid, int value)
{
  if (valid)
    printf("%10x", value);
  else
    printf("         x");
}

static void dump_column(bool valid, const char* value)
{
  if (valid)
    printf("%10s", value);
  else
    printf("         x");
}

static const char* animation_name(const bim::game::context& context,
                                  entt::entity e,
                                  const entt::registry& registry)
{
  const bim::game::player_animations& animations =
      context.get<const bim::game::player_animations>();

  const bim::game::animation_state* const animation =
      registry.try_get<bim::game::animation_state>(e);

  if (!animation)
    return "none";

  if (animation->model == animations.idle_down)
    return "idle_down";

  if (animation->model == animations.idle_left)
    return "idle_left";

  if (animation->model == animations.idle_right)
    return "idle_right";

  if (animation->model == animations.idle_up)
    return "idle_up";

  if (animation->model == animations.walk_down)
    return "walk_down";

  if (animation->model == animations.walk_left)
    return "walk_deft";

  if (animation->model == animations.walk_right)
    return "walk_right";

  if (animation->model == animations.walk_up)
    return "walk_up";

  if (animation->model == animations.burn)
    return "burn";

  if (animation->model == animations.die)
    return "die";

  return "unknown";
}

void bim::game::dump_arena(const arena& arena,
                           const entity_world_map& entity_map,
                           const context& context,
                           const entt::registry& registry)
{
  const int w = arena.width();
  const int h = arena.height();

  std::vector<std::vector<std::string>> arena_str(
      h, std::vector<std::string>(w, " "));

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

  const entt::registry::storage_for_type<solid>* const solids =
      registry.storage<solid>();

  for (int y = 0; y != h; ++y)
    for (int x = 0; x != w; ++x)
      {
        const std::span<const entt::entity> entities =
            entity_map.entities_at(x, y);

        if (arena.is_static_wall(x, y))
          arena_str[y][x] = "▒";
        else
          {
            static const char* const e = " 123456789abcdef";
            arena_str[y][x] = std::string(e + (int)arena.fences(x, y), 1);
          }

        if (!entities.empty())
          arena_str[y][x] = "!";

        if (solids)
          for (entt::entity e : entities)
            if (solids->contains(e))
              {
                arena_str[y][x] = "░";
                break;
              }
      }

  std::vector<player> players;
  std::array<bool, g_max_player_count> valid{};
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

        valid[p.index] = true;
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

  registry.view<invisibility_power_up, position_on_grid>().each(
      [&](position_on_grid pos) -> void
      {
        arena_str[pos.y][pos.x] = "I";
      });

  registry.view<shield_power_up, position_on_grid>().each(
      [&](position_on_grid pos) -> void
      {
        arena_str[pos.y][pos.x] = "S";
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
      {
        printf("%*c", w + 2, eol);
        return;
      }

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
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i], (int)players[i].index);
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "player_entity");
  for (int i = 0; i != player_count; ++i)
    dump_column_hex(valid[i], (int)player_entities[i]);
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "animation");
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i],
                animation_name(context, player_entities[i], registry));
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "bomb_capacity");
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i], (int)players[i].bomb_capacity);
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "bomb_available");
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i], (int)players[i].bomb_available);
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "bomb_strength");
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i], (int)players[i].bomb_strength);
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "invisible");
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i], (int)is_invisible(registry, player_entities[i]));
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "shield");
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i], (int)has_shield(registry, player_entities[i]));
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "invincible");
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i], (int)is_invincible(registry, player_entities[i]));
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "x");
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i], player_position[i].x_float());
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "y");
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i], player_position[i].y_float());
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "x_grid");
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i], (int)player_position[i].grid_aligned_x());
  printf("\n");

  print_arena_line(' ');
  printf("%-15s", "y_grid");
  for (int i = 0; i != player_count; ++i)
    dump_column(valid[i], (int)player_position[i].grid_aligned_y());
  printf("\n");

  std::array<player_action, g_max_player_count> queued_actions;

  registry.view<player, player_action>().each(
      [&queued_actions](const player& p, const player_action& action)
      {
        queued_actions[p.index] = action;
      });

  const auto print_actions =
      [&, player_count](
          const char* title,
          const std::array<player_action, g_max_player_count>& actions) -> void
  {
    print_arena_line(' ');
    printf("%-15s", title);
    for (int i = 0; i != player_count; ++i)
      if (!valid[i])
        printf("       xx");
      else
        {
          char d = '.';

          switch (actions[i].movement)
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

          printf("       %c%c", d, actions[i].drop_bomb ? 'b' : '.');
        }
    printf("\n");
  };

  print_actions("queued actions", queued_actions);

  std::array<player_action, g_max_player_count> player_actions;

  registry.view<player, player_action_queue>().each(
      [&player_actions](const player& p, const player_action_queue& queue)
      {
        player_actions[p.index] = queue.m_queue[0].action;
      });

  print_actions("action", player_actions);

  std::array<bool, g_max_player_count> kick_events{};
  bool print_kick_events = false;

  registry.view<player, kicked>().each(
      [&kick_events, &print_kick_events](const player& p)
      {
        kick_events[p.index] = true;
        print_kick_events = true;
      });

  if (print_kick_events)
    {
      print_arena_line(' ');
      printf("%-15s", "kicked");
      for (int i = 0; i != player_count; ++i)
        printf("%9d", (int)kick_events[i]);
      printf("\n");
    }

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

  while (arena_print_y != h)
    print_arena_line('\n');

  int bomb_lines = 0;
  if (!bombs.empty())
    {
      printf("bombs(x, y, player, strength): ");
      while (bomb_print_i != bombs.size())
        {
          print_bombs_line();
          ++bomb_lines;
        }
    }

  // Try to keep a fixed number of lines after the arena, it makes paging
  // easier.
  for (; bomb_lines < 3; ++bomb_lines)
    printf("\n");
}
