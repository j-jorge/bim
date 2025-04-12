// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/contest.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/check_game_over.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_direction.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/contest_fingerprint.hpp>
#include <bim/game/contest_result.hpp>
#include <bim/game/factory/arena_reduction.hpp>
#include <bim/game/factory/fog_of_war.hpp>
#include <bim/game/factory/main_timer.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/feature_flags.hpp>
#include <bim/game/system/apply_player_action.hpp>
#include <bim/game/system/arena_reduction.hpp>
#include <bim/game/system/fog_of_war_updater.hpp>
#include <bim/game/system/refresh_bomb_inventory.hpp>
#include <bim/game/system/remove_dead_objects.hpp>
#include <bim/game/system/update_bomb_power_up_spawners.hpp>
#include <bim/game/system/update_bomb_power_ups.hpp>
#include <bim/game/system/update_bombs.hpp>
#include <bim/game/system/update_brick_walls.hpp>
#include <bim/game/system/update_falling_blocks.hpp>
#include <bim/game/system/update_flame_power_up_spawners.hpp>
#include <bim/game/system/update_flame_power_ups.hpp>
#include <bim/game/system/update_flames.hpp>
#include <bim/game/system/update_players.hpp>
#include <bim/game/system/update_timers.hpp>

#include <bim/game/level_generation.hpp>
#include <bim/game/random_generator.hpp>

#include <bim/assume.hpp>

#include <entt/entity/registry.hpp>

#include <boost/random/uniform_int_distribution.hpp>

constexpr std::chrono::milliseconds bim::game::contest::tick_interval;

static void add_players(entt::registry& registry, std::uint8_t player_count,
                        std::uint8_t arena_width, std::uint8_t arena_height)
{
  bim_assume(player_count > 0);

  const bim::game::position_on_grid player_start_position[] = {
    bim::game::position_on_grid(1, 1),
    bim::game::position_on_grid(arena_width - 2, arena_height - 2),
    bim::game::position_on_grid(arena_width - 2, 1),
    bim::game::position_on_grid(1, arena_height - 2)
  };
  const int start_position_count = std::size(player_start_position);

  for (std::size_t i = 0; i != player_count; ++i)
    {
      const int start_position_index = i % start_position_count;
      bim::game::player_factory(registry, i,
                                player_start_position[start_position_index].x,
                                player_start_position[start_position_index].y);
    }
}

static void add_fog_of_war(entt::registry& registry, std::uint8_t player_count,
                           std::uint8_t arena_width, std::uint8_t arena_height,
                           std::uint64_t seed)
{
  // We use an independent random number generator for the fog. The server does
  // not generate it so using the main one would cause a different set-up for
  // the clients and the server.
  bim::game::random_generator random(seed);

  // Avoid removing cells on the borders as well as on the 9 cells in each
  // corner (where the players are located by default). The removed cells are
  // the same for all players.
  int available_cells = (arena_width - 2) * (arena_height - 2) - 4 * 9;
  constexpr int max_excluded_cell_count = 3;
  int excluded_cell_count = 0;
  std::array<bim::game::position_on_grid, max_excluded_cell_count> excluded;

  using uniform_u8_distribution =
      boost::random::uniform_int_distribution<std::uint8_t>;

  for (int cells_to_exclude =
               std::min(max_excluded_cell_count, available_cells),
           i = 0;
       (cells_to_exclude != 0) && (i < arena_width * arena_height); ++i)
    {
      uniform_u8_distribution d(1, available_cells);

      if (d(random) > cells_to_exclude)
        --available_cells;
      else
        {
          const int x = uniform_u8_distribution(1, arena_width - 2)(random);
          const int y = uniform_u8_distribution(1, arena_height - 2)(random);
          bool keep = true;

          // Keep at least one cell between two holes.
          for (int j = 0; (j != excluded_cell_count) && keep; ++j)
            keep = (std::abs(x - excluded[j].x) > 1)
                   || (std::abs(y - excluded[j].y) > 1);

          // Avoid removing the fog near the default player positions.
          if (keep
              && ((x >= 4 && x < arena_width - 4)
                  || (y >= 4 && y < arena_height - 4)))
            {
              excluded[excluded_cell_count].x = x;
              excluded[excluded_cell_count].y = y;
              ++excluded_cell_count;
              --available_cells;
              --cells_to_exclude;
            }
        }
    }

  for (int player_index = 0; player_index != player_count; ++player_index)
    fog_of_war_factory(registry, player_index, arena_width, arena_height,
                       std::span(excluded.begin(), excluded_cell_count));
}

bim::game::contest::contest(const contest_fingerprint& fingerprint)
  : contest(fingerprint, fingerprint.player_count)
{}

bim::game::contest::contest(const contest_fingerprint& fingerprint,
                            std::uint8_t local_player_index)
  : m_registry(new entt::registry())
  , m_arena(new bim::game::arena(fingerprint.arena_width,
                                 fingerprint.arena_height))
{
  add_players(*m_registry, fingerprint.player_count, fingerprint.arena_width,
              fingerprint.arena_height);
  generate_basic_level_structure(*m_arena);

  bim::game::random_generator random(fingerprint.seed);

  insert_random_brick_walls(*m_arena, *m_registry, random,
                            fingerprint.brick_wall_probability);

  if (!!(fingerprint.features & feature_flags::falling_blocks))
    arena_reduction_factory(*m_registry, std::chrono::minutes(2));
  else
    main_timer_factory(*m_registry, std::chrono::minutes(3));

  // The fog of war is a local feature, it has no impact on simulations run
  // elsewhere. Consequently we instantiate it only if there is a local player.
  if ((local_player_index < fingerprint.player_count)
      && !!(fingerprint.features & feature_flags::fog_of_war))
    add_fog_of_war(*m_registry, fingerprint.player_count,
                   fingerprint.arena_width, fingerprint.arena_height,
                   fingerprint.seed);

  m_arena_reduction.reset(new arena_reduction(*m_arena));
  m_fog_of_war.reset(
      new fog_of_war_updater(*m_registry, *m_arena, fingerprint.player_count));
}

bim::game::contest::~contest() = default;

const bim::table_2d<bim::game::fog_of_war*>&
bim::game::contest::fog_map(std::size_t player_index) const
{
  return m_fog_of_war->fog(player_index);
}

bim::game::contest_result bim::game::contest::tick()
{
  refresh_bomb_inventory(*m_registry);
  update_timers(*m_registry, tick_interval);
  apply_player_action(*m_registry, *m_arena);
  m_arena_reduction->update(*m_registry, *m_arena);
  update_falling_blocks(*m_registry, *m_arena);
  update_bombs(*m_registry, *m_arena);
  update_flames(*m_registry, *m_arena);
  update_brick_walls(*m_registry, *m_arena);
  update_bomb_power_up_spawners(*m_registry, *m_arena);
  update_bomb_power_ups(*m_registry, *m_arena);
  update_flame_power_up_spawners(*m_registry, *m_arena);
  update_flame_power_ups(*m_registry, *m_arena);
  update_players(*m_registry, *m_arena);
  m_fog_of_war->update(*m_registry);

  remove_dead_objects(*m_registry);

  return check_game_over(*m_registry);
}

entt::registry& bim::game::contest::registry()
{
  return *m_registry;
}

const entt::registry& bim::game::contest::registry() const
{
  return *m_registry;
}

const bim::game::arena& bim::game::contest::arena() const
{
  return *m_arena;
}

void bim::game::contest::arena(const bim::game::arena& a)
{
  *m_arena = a;
}
