// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/bot.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/crate.hpp>
#include <bim/game/component/falling_block.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/fog_of_war.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/invisibility_power_up.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/component/power_up.hpp>
#include <bim/game/component/shield.hpp>
#include <bim/game/component/shield_power_up.hpp>
#include <bim/game/component/solid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/constant/bomb_near_explosion_duration.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/player_animations.hpp>
#include <bim/game/entity_world_map.hpp>
#include <bim/game/static_wall.hpp>
#include <bim/game/system/move_player.hpp>

#include <bim/assume.hpp>
#include <bim/table_2d.impl.hpp>
#include <bim/tracy.hpp>

#include <entt/core/type_info.hpp>
#include <entt/entity/registry.hpp>

static const entt::id_type g_type_void = entt::type_index<void>::value();
static const entt::id_type g_type_bomb_power_up =
    entt::type_index<bim::game::bomb_power_up>::value();
static const entt::id_type g_type_flame_power_up =
    entt::type_index<bim::game::flame_power_up>::value();
static const entt::id_type g_type_shield_power_up =
    entt::type_index<bim::game::shield_power_up>::value();
static const entt::id_type g_type_invisibility_power_up =
    entt::type_index<bim::game::invisibility_power_up>::value();

bim::game::bot::goal::goal(int x, int y)
  : target(x, y)
  , drop_bomb(false)
  , crate_count(0)
  , power_up(false)
  , opponent_in_danger(false)
{}

bim::game::bot::bot(std::uint8_t player_index, std::uint8_t arena_width,
                    std::uint8_t arena_height, std::uint64_t seed)
  : m_visibility_map(arena_width, arena_height)
  , m_crate_map(arena_width, arena_height)
  , m_solid_map(arena_width, arena_height)
  , m_immediate_danger_map(arena_width, arena_height)
  , m_danger_map(arena_width, arena_height)
  , m_power_up_map(arena_width, arena_height)
  , m_distance(arena_width, arena_height)
  , m_previous_cell(arena_width, arena_height)
  , m_random(seed)
  , m_reach_for_safety(false)
  , m_player_index(player_index)
{
  m_candidate_goals.reserve(arena_width + arena_height);
  m_goal_path.reserve(arena_width * arena_height);
}

bim::game::bot::bot(const bot&) noexcept = default;
bim::game::bot::bot(bot&&) noexcept = default;
bim::game::bot::~bot() = default;

bim::game::bot& bim::game::bot::operator=(const bot&) noexcept = default;
bim::game::bot& bim::game::bot::operator=(bot&&) noexcept = default;

std::uint8_t bim::game::bot::player_index() const
{
  return m_player_index;
}

bim::game::player_action bim::game::bot::think(const contest& contest)
{
  ZoneScoped;

  per_player_array<bool> opponent_is_valid;
  per_player_array<position_on_grid> player_positions;
  const player* bot_player;
  const player_action_queue* queued_bot_actions;
  fractional_position_on_grid bot_fractional_position;

  find_players(contest, opponent_is_valid, player_positions, bot_player,
               queued_bot_actions, bot_fractional_position);

  if (!bot_player)
    return player_action{};

  bim_assume(queued_bot_actions);

  const int unknown_count = build_visibility_map(contest);
  const int crate_count = build_crate_map(contest);
  build_solid_map(contest);
  build_danger_map(contest, *bot_player, *queued_bot_actions);
  build_power_up_map(contest);

  const position_on_grid& bot_position = player_positions[m_player_index];

  if (m_reach_for_safety)
    {
      if (!m_immediate_danger_map(bot_position.x, bot_position.y))
        {
          // We are in a safe place now, we can search another goal.
          m_reach_for_safety = false;
          m_goal = std::nullopt;
        }
    }
  else
    {
      if (m_immediate_danger_map(bot_position.x, bot_position.y))
        {
          // We are in danger, we need to find a safe place quickly.
          m_reach_for_safety = true;
          m_goal = std::nullopt;
        }
    }

  if (!m_goal || !goal_is_feasible(contest, *bot_player))
    {
      if (!find_goal(contest, opponent_is_valid, player_positions, *bot_player,
                     crate_count, unknown_count))
        return player_action{};

      store_goal_path();
    }

  return action_toward_goal(contest, *bot_player, *queued_bot_actions,
                            opponent_is_valid, player_positions,
                            bot_fractional_position);
}

void bim::game::bot::find_players(
    const contest& contest, per_player_array<bool>& opponent_is_valid,
    per_player_array<position_on_grid>& player_positions,
    const player*& bot_player, const player_action_queue*& queued_actions,
    fractional_position_on_grid& bot_fractional_position) const
{
  bot_player = nullptr;
  queued_actions = nullptr;
  opponent_is_valid.fill(false);

  const player_animations& animations =
      contest.context().get<const player_animations>();

  const entt::registry& registry = contest.registry();

  for (const auto& [entity, player, position, queue, state] :
       registry
           .view<player, fractional_position_on_grid, player_action_queue,
                 animation_state>()
           .each())
    {
      player_positions[player.index].x = position.grid_aligned_x();
      player_positions[player.index].y = position.grid_aligned_y();

      const bool alive = animations.is_alive(state.model);

      if (player.index != m_player_index)
        opponent_is_valid[player.index] = alive;
      else if (alive)
        {
          bot_player = &player;
          queued_actions = &queue;
          bot_fractional_position = position;
        }
    }
}

int bim::game::bot::build_visibility_map(const contest& contest)
{
  int result = 0;

  const bim::table_2d<fog_of_war*>& fog_map = contest.fog_map(m_player_index);

  assert(fog_map.width() == m_visibility_map.width());
  assert(fog_map.height() == m_visibility_map.height());

  for (std::size_t y = 0, h = fog_map.height(); y != h; ++y)
    for (std::size_t x = 0, w = fog_map.width(); x != w; ++x)
      {
        const fog_of_war* const fog = fog_map(x, y);
        const bool visible = !fog
                             || ((fog->state == fog_state::hiding)
                                 && (fog->opacity < fog_of_war::full_opacity));
        m_visibility_map(x, y) = visible;
        result += !visible;
      }

  return result;
}

int bim::game::bot::build_crate_map(const contest& contest)
{
  m_crate_map.fill(false);
  int result = 0;

  contest.registry().view<crate, position_on_grid>().each(
      [this, &result](const position_on_grid& p)
        {
          if (m_visibility_map(p.x, p.y))
            {
              m_crate_map(p.x, p.y) = true;
              ++result;
            }
        });

  return result;
}

void bim::game::bot::build_solid_map(const contest& contest)
{
  m_solid_map.fill(false);

  contest.registry().view<solid, position_on_grid>().each(
      [this](const position_on_grid& p)
        {
          m_solid_map(p.x, p.y) = m_visibility_map(p.x, p.y);
        });
}

void bim::game::bot::build_danger_map(
    const contest& contest, const player& bot_player,
    const player_action_queue& queued_actions)
{
  // If we can't tell what's in the cell then its a potential daner.
  for (std::size_t y = 0, h = m_visibility_map.height(); y != h; ++y)
    for (std::size_t x = 0, w = m_visibility_map.width(); x != w; ++x)
      {
        m_danger_map(x, y) = !m_visibility_map(x, y);
        m_immediate_danger_map(x, y) = !m_visibility_map(x, y);
      }

  contest.registry().view<flame, position_on_grid>().each(
      [this](const flame&, const position_on_grid& p)
        {
          m_danger_map(p.x, p.y) = true;
          m_immediate_danger_map(p.x, p.y) = true;
        });

  // Insert planned bomb droppings.
  for (std::size_t i = 0; i != player_action_queue::queue_size; ++i)
    if (queued_actions.m_queue[i].action.drop_bomb)
      store_bomb_danger(m_danger_map, contest,
                        position_on_grid(queued_actions.m_queue[i].arena_x,
                                         queued_actions.m_queue[i].arena_y),
                        bot_player.bomb_strength);

  contest.registry().view<bomb, position_on_grid, timer>().each(
      [this, &contest](const bomb& b, const position_on_grid& p,
                       const timer& t)
        {
          store_bomb_danger(m_danger_map, contest, p, b.strength);

          if (t.duration <= g_bomb_near_explosion_duration)
            store_bomb_danger(m_immediate_danger_map, contest, p, b.strength);
        });

  // Falling blocks are a danger.
  contest.registry().view<falling_block, position_on_grid>().each(
      [this](const position_on_grid& p)
        {
          m_danger_map(p.x, p.y) = true;
          m_immediate_danger_map(p.x, p.y) = true;
        });
}

void bim::game::bot::build_power_up_map(const contest& contest)
{
  m_power_up_map.fill(g_type_void);

  const entt::registry& registry = contest.registry();

  const entt::registry::storage_for_type<bomb_power_up>* const bomb_power_ups =
      registry.storage<bomb_power_up>();
  const entt::registry::storage_for_type<flame_power_up>* const
      flame_power_ups = registry.storage<flame_power_up>();
  const entt::registry::storage_for_type<shield_power_up>* const
      shield_power_ups = registry.storage<shield_power_up>();
  const entt::registry::storage_for_type<invisibility_power_up>* const
      invisibility_power_ups = registry.storage<invisibility_power_up>();

  const auto contains = [&](const auto* storage, entt::entity e) -> bool
    {
      return storage && storage->contains(e);
    };

  contest.registry().view<power_up, position_on_grid>().each(
      [&](entt::entity e, const position_on_grid& p)
        {
          if (!m_visibility_map(p.x, p.y))
            return;

          if (contains(bomb_power_ups, e))
            m_power_up_map(p.x, p.y) = g_type_bomb_power_up;
          else if (contains(flame_power_ups, e))
            m_power_up_map(p.x, p.y) = g_type_flame_power_up;
          else if (contains(shield_power_ups, e))
            m_power_up_map(p.x, p.y) = g_type_shield_power_up;
          else if (contains(invisibility_power_ups, e))
            m_power_up_map(p.x, p.y) = g_type_invisibility_power_up;
        });
}

void bim::game::bot::store_bomb_danger(bim::table_2d<bool>& map,
                                       const contest& contest,
                                       const position_on_grid& p, int d)
{
  map(p.x, p.y) = true;

  simulate_bomb_explosion(contest, d, p.x, p.y,
                          [this, &map](int x, int y)
                            {
                              map(x, y) = true;
                              return m_solid_map(x, y);
                            });
}

bool bim::game::bot::goal_is_feasible(const contest& contest,
                                      const player& bot_player) const
{
  if (m_goal->power_up)
    return m_power_up_map(m_goal->target.x, m_goal->target.y) != g_type_void;

  if (!m_goal->crate_count)
    return true;

  int crate_count = 0;

  // Returns true if the bomb simulation must stop in this direction, false to
  // continue with the next cell.
  const auto count_crate = [&](int x, int y)
    {
      if (m_power_up_map(x, y) != g_type_void)
        // There's a power up in the cell, it's going to stop the flame.
        return true;

      if (!m_crate_map(x, y))
        // No crate means no obstacle, keep going.
        return false;

      // A crate in the danger zone is already going to burn, so we don't
      // count it.
      if (!m_danger_map(x, y))
        ++crate_count;

      return true;
    };

  simulate_bomb_explosion(contest, bot_player.bomb_strength, m_goal->target.x,
                          m_goal->target.y, count_crate);

  return crate_count == m_goal->crate_count;
}

bool bim::game::bot::find_goal(
    const contest& contest, const per_player_array<bool>& opponent_is_valid,
    const per_player_array<position_on_grid>& player_positions,
    const player& bot_player, int crate_count, int unknown_count)
{
  m_goal = std::nullopt;

  const position_on_grid& bot_position = player_positions[m_player_index];

  m_navigation.paths(m_distance, m_previous_cell, contest.registry(),
                     contest.arena(), contest.entity_map(), bot_position.x,
                     bot_position.y, m_visibility_map);

  if (m_reach_for_safety)
    return find_safety_goal(contest, bot_position);

  if ((crate_count > 0) || (unknown_count > 0))
    return find_exploration_goal(contest, opponent_is_valid, player_positions,
                                 bot_player);

  return find_attack_goal(contest, opponent_is_valid, player_positions,
                          bot_player);
}

bool bim::game::bot::find_safety_goal(const contest& contest,
                                      const position_on_grid& bot_position)
{
  m_candidate_goals.clear();
  int shortest_distance = navigation_check::unreachable;

  // Find the nearest cell in which there is no danger.
  for (std::size_t y = 0, h = m_distance.height(); y != h; ++y)
    for (std::size_t x = 0, w = m_distance.width(); x != w; ++x)
      {
        const int distance = m_distance(x, y);

        if ((distance > shortest_distance)
            || (distance >= navigation_check::unreachable)
            || m_danger_map(x, y))
          continue;

        if (distance == shortest_distance)
          {
            m_candidate_goals.emplace_back(x, y);
            continue;
          }

        shortest_distance = distance;
        m_candidate_goals.clear();
        m_candidate_goals.emplace_back(x, y);
      }

  if (!m_candidate_goals.empty())
    {
      std::uniform_int_distribution<std::size_t> d(0, m_candidate_goals.size()
                                                          - 1);
      m_goal = m_candidate_goals[d(m_random)];
    }

  return !!m_goal;
}

bool bim::game::bot::find_exploration_goal(
    const contest& contest, const per_player_array<bool>& opponent_is_valid,
    const per_player_array<position_on_grid>& player_positions,
    const player& bot_player)
{
  const entt::registry& registry = contest.registry();

  bool in_open_area = false;
  bool has_shield = false;

  {
    const player_animations& animations =
        contest.context().get<const player_animations>();

    for (const auto& [entity, player, position, state] :
         registry.view<player, fractional_position_on_grid, animation_state>()
             .each())
      if (player.index == m_player_index)
        has_shield = bim::game::has_shield(registry, entity);
      else if (animations.is_alive(state.model))
        {
          const std::size_t x = position.grid_aligned_x();
          const std::size_t y = position.grid_aligned_y();

          in_open_area |=
              m_visibility_map(x, y)
              && (m_distance(x, y) < navigation_check::unreachable);
        }
  }

  m_candidate_goals.clear();
  int best_reward = std::numeric_limits<int>::min();

  // Check what can be done in each reachable cell and select the most
  // interesting goal.
  for (std::size_t y = 0, h = m_distance.height(); y != h; ++y)
    for (std::size_t x = 0, w = m_distance.width(); x != w; ++x)
      {
        if (m_distance(x, y) >= navigation_check::unreachable)
          continue;

        int cell_reward;
        const goal g = analyze_cell(cell_reward, contest, opponent_is_valid,
                                    player_positions, bot_player, has_shield,
                                    in_open_area, x, y);

        if (cell_reward < best_reward)
          continue;

        if (cell_reward == best_reward)
          {
            m_candidate_goals.push_back(g);
            continue;
          }

        best_reward = cell_reward;
        m_candidate_goals.clear();
        m_candidate_goals.push_back(g);
      }

  if (!m_candidate_goals.empty())
    {
      std::uniform_int_distribution<std::size_t> d(0, m_candidate_goals.size()
                                                          - 1);
      m_goal = m_candidate_goals[d(m_random)];
    }

  return !!m_goal;
}

bool bim::game::bot::find_attack_goal(
    const contest& contest, const per_player_array<bool>& opponent_is_valid,
    const per_player_array<position_on_grid>& player_positions,
    const player& bot_player)
{
  // Find the nearest opponent.
  std::size_t opponent_index = opponent_is_valid.size();
  std::uint8_t opponent_distance = navigation_check::unreachable;

  for (std::size_t i = 0; i != opponent_is_valid.size(); ++i)
    if (opponent_is_valid[i])
      {
        const position_on_grid& p = player_positions[i];

        if (!m_visibility_map(p.x, p.y))
          continue;

        const std::uint8_t d = m_distance(p.x, p.y);

        if (d < opponent_distance)
          {
            opponent_distance = d;
            opponent_index = i;
          }
      }

  if (opponent_index == opponent_is_valid.size())
    return false;

  // Run toward the opponent.
  m_goal.emplace(player_positions[opponent_index].x,
                 player_positions[opponent_index].y);
  m_goal->drop_bomb = true;

  return true;
}

bim::game::bot::goal bim::game::bot::analyze_cell(
    int& reward, const contest& contest,
    const per_player_array<bool>& opponent_is_valid,
    const per_player_array<position_on_grid>& player_positions,
    const player& bot_player, bool bot_has_shield, bool bot_in_open_area,
    std::size_t x, std::size_t y) const
{
  constexpr int uncover_reward = 10;
  constexpr int crate_reward = 20;
  constexpr int shield_reward = 80;
  constexpr int invisibility_reward = 80;
  constexpr int power_up_fallback_reward = 40;
  constexpr int opponent_in_danger_reward = 50;
  constexpr int burning_power_up_penalty = 3 * crate_reward;
  constexpr int danger_penalty = 1000;

  int power_up_reward = 0;
  const entt::id_type power_up = m_power_up_map(x, y);

  if ((power_up == g_type_bomb_power_up)
      || (power_up == g_type_flame_power_up))
    power_up_reward += power_up_fallback_reward;
  else if (!bot_has_shield && (power_up == g_type_shield_power_up))
    power_up_reward += shield_reward;
  else if (bot_in_open_area && (power_up == g_type_invisibility_power_up))
    power_up_reward += invisibility_reward;

  int uncover_count = 0;

  for (int dy = -1; dy <= 1; ++dy)
    for (int dx = -1; dx <= 1; ++dx)
      uncover_count += !m_visibility_map(x + dx, y + dy);

  goal goal(x, y);
  int crate_count;
  // Power ups we know we are going to burn.
  int burning_power_up_count;
  // Power ups that may come out of a burning crate after we have dropped a
  // bomb.
  int burning_power_up_risk_count;
  int opponents_in_danger;

  // Evaluate the impact of dropping a bomb.
  evaluate_bomb_explosion(contest, bot_player.bomb_strength, x, y,
                          opponent_is_valid, player_positions, crate_count,
                          burning_power_up_count, burning_power_up_risk_count,
                          opponents_in_danger);

  goal.drop_bomb = !power_up_reward
                   && ((crate_count > 0) || (opponents_in_danger > 0))
                   && (burning_power_up_risk_count == 0);
  goal.crate_count = crate_count;
  goal.power_up = power_up_reward;
  goal.opponent_in_danger = opponents_in_danger;

  const int clipped_distance =
      std::max(1, std::min((int)m_distance(goal.target.x, goal.target.y), 10));

  reward = ((power_up_reward ? power_up_reward : (crate_reward * crate_count))
            + uncover_count * uncover_reward
            + opponents_in_danger * opponent_in_danger_reward
            - burning_power_up_penalty * burning_power_up_count)
               / clipped_distance
           - danger_penalty * m_danger_map(goal.target.x, goal.target.y);

  return goal;
}

void bim::game::bot::store_goal_path()
{
  bim_assume(!!m_goal);

  m_goal_step = 0;
  m_goal_path.clear();

  position_on_grid p = m_goal->target;
  m_goal_path.emplace_back(p);

  while (m_previous_cell(p.x, p.y) != p)
    {
      p = m_previous_cell(p.x, p.y);
      m_goal_path.push_back(p);
    }

  std::reverse(m_goal_path.begin(), m_goal_path.end());
}

bim::game::player_action bim::game::bot::action_toward_goal(
    const contest& contest, const player& bot_player,
    const player_action_queue& queued_actions,
    const per_player_array<bool>& opponent_is_valid,
    const per_player_array<position_on_grid>& player_positions,
    const fractional_position_on_grid& bot_fractional_position)
{
  // Already completed.
  if (m_goal_step == m_goal_path.size())
    return {};

  const position_on_grid& bot_position = player_positions[m_player_index];

  // We have reached the current step, switch to the next.
  if (bot_position == m_goal_path[m_goal_step])
    {
      ++m_goal_step;

      if (m_goal_step != m_goal_path.size())
        {
          // If we are going straight into danger, then don't move.
          if (!m_immediate_danger_map(m_goal_path[m_goal_step - 1].x,
                                      m_goal_path[m_goal_step - 1].y)
              && m_immediate_danger_map(m_goal_path[m_goal_step].x,
                                        m_goal_path[m_goal_step].y))
            {
              --m_goal_step;
              return player_action{};
            }

          if (is_a_good_place_to_drop_a_bomb(
                  contest, bot_player, queued_actions, opponent_is_valid,
                  player_positions, bot_fractional_position))
            // If it's a good idea to drop a bomb on the path, then do it.
            return player_action{ .movement = player_movement::idle,
                                  .drop_bomb = true };
        }
      else
        {
          const bool drop_bomb =
              m_goal->drop_bomb && (bot_player.bomb_available > 0)
              && simulate_bomb_drop(contest, bot_player, queued_actions,
                                    bot_fractional_position);

          m_goal = std::nullopt;

          return player_action{ .movement = player_movement::idle,
                                .drop_bomb = drop_bomb };
        }
    }

  player_action result{};

  if (bot_position.x < m_goal_path[m_goal_step].x)
    result.movement = player_movement::right;
  else if (bot_position.x > m_goal_path[m_goal_step].x)
    result.movement = player_movement::left;
  else if (bot_position.y < m_goal_path[m_goal_step].y)
    result.movement = player_movement::down;
  else
    result.movement = player_movement::up;

  return result;
}

/// Tell if it's a good place to drop a bomb on the way toward the goal.
bool bim::game::bot::is_a_good_place_to_drop_a_bomb(
    const contest& contest, const player& bot_player,
    const player_action_queue& queued_actions,
    const per_player_array<bool>& opponent_is_valid,
    const per_player_array<position_on_grid>& player_positions,
    const fractional_position_on_grid& bot_fractional_position)
{
  // We must keep a bomb for the target.
  if (bot_player.bomb_available <= m_goal->drop_bomb)
    return false;

  // Flush the pending actions to get the final position of the
  // bot, that it will reach after having dropped the bomb.
  fractional_position_on_grid fractional_position = bot_fractional_position;
  flush_bot_actions(contest, queued_actions, fractional_position);

  const position_on_grid final_position(fractional_position.grid_aligned_x(),
                                        fractional_position.grid_aligned_y());

  // Don't drop a bomb if it puts our target position in the danger zone.
  if (((m_goal->target.y == final_position.y)
       && (std::abs(m_goal->target.x - final_position.x)
           <= bot_player.bomb_strength))
      || ((m_goal->target.x == final_position.x)
          && (std::abs(m_goal->target.y - final_position.y)
              <= bot_player.bomb_strength)))
    return false;

  int crate_count;
  // Power ups we know we are going to burn.
  int burning_power_up_count;
  // Power ups that may come out of a burning crate after we have dropped a
  // bomb.
  int burning_power_up_risk_count;
  int opponents_in_danger;

  evaluate_bomb_explosion(
      contest, bot_player.bomb_strength, final_position.x, final_position.y,
      opponent_is_valid, player_positions, crate_count, burning_power_up_count,
      burning_power_up_risk_count, opponents_in_danger);

  if (burning_power_up_count || burning_power_up_risk_count
      || (!crate_count && !opponents_in_danger))
    return false;

  store_bomb_danger(m_danger_map, contest, final_position,
                    bot_player.bomb_strength);

  // We must go one cell further than the bomb's reach.
  const int safe_distance = bot_player.bomb_strength + 1;

  // It's safe if there's a path toward a safe place.
  return m_navigation.exists(contest.registry(), contest.arena(),
                             contest.entity_map(), final_position.x,
                             final_position.y, safe_distance, m_danger_map);
}

bool bim::game::bot::simulate_bomb_drop(
    const contest& contest, const player& bot_player,
    const player_action_queue& queued_actions,
    const fractional_position_on_grid& bot_fractional_position)
{
  // Flush the pending actions to get the final position of the
  // bot, that it will reach after having dropped the bomb.
  fractional_position_on_grid fractional_position = bot_fractional_position;
  flush_bot_actions(contest, queued_actions, fractional_position);

  const position_on_grid final_position(fractional_position.grid_aligned_x(),
                                        fractional_position.grid_aligned_y());

  store_bomb_danger(m_danger_map, contest, final_position,
                    bot_player.bomb_strength);

  // We must go one cell further than the bomb's reach.
  const int safe_distance = bot_player.bomb_strength + 1;

  // It's safe if there's a path toward a safe place.
  return m_navigation.exists(contest.registry(), contest.arena(),
                             contest.entity_map(), final_position.x,
                             final_position.y, safe_distance, m_danger_map);
}

void bim::game::bot::flush_bot_actions(
    const contest& contest, const player_action_queue& queued_actions,
    fractional_position_on_grid& bot_fractional_position)
{
  const entt::registry& registry = contest.registry();
  const entity_world_map& entity_map = contest.entity_map();
  const arena& arena = contest.arena();

  for (std::size_t i = 0; i != player_action_queue::queue_size; ++i)
    move_player(bot_fractional_position,
                queued_actions.m_queue[i].action.movement, registry, arena,
                entity_map);
}

/**
 * \param burning_power_up_count Power ups we know we are going to burn.
 * \param burning_power_up_risk_count Power ups that may come out of a burning
 * crate after we have dropped a bomb.
 */
void bim::game::bot::evaluate_bomb_explosion(
    const contest& contest, int blast_distance, int x, int y,
    const per_player_array<bool>& opponent_is_valid,
    const per_player_array<position_on_grid>& player_positions,
    int& crate_count, int& burning_power_up_count,
    int& burning_power_up_risk_count, int& opponents_in_danger) const
{
  crate_count = 0;
  burning_power_up_count = 0;
  burning_power_up_risk_count = 0;
  opponents_in_danger = 0;

  // Returns true if the bomb simulation must stop in this direction, false to
  // continue with the next cell.
  const auto count_crate = [&](int x, int y)
    {
      if (m_power_up_map(x, y) != g_type_void)
        {
          // There's a power up in the cell, it's going to stop the flame.
          ++burning_power_up_count;
          return true;
        }

      // The players don't stop the flames.
      for (std::size_t i = 0; i != opponent_is_valid.size(); ++i)
        if (opponent_is_valid[i] && (player_positions[i].x == x)
            && (player_positions[i].y == y))
          ++opponents_in_danger;

      if (!m_crate_map(x, y))
        // No crate means no obstacle, keep going.
        return false;

      if (m_distance(x, y) != navigation_check::border)
        // There's a crate and it's not on the border of the reachable area,
        // thus we can't reach it nor its potential power up. We stop the flame
        // here.
        return true;

      if (m_danger_map(x, y))
        // A crate in the danger zone is already going to burn. If we drop
        // another bomb for it we may burn its potential power up.
        ++burning_power_up_risk_count;
      else
        ++crate_count;

      return true;
    };

  simulate_bomb_explosion(contest, blast_distance, x, y, count_crate);
}

template <typename Visit>
void bim::game::bot::simulate_bomb_explosion(const contest& contest,
                                             int blast_distance, int x, int y,
                                             Visit&& visit) const
{
  const int min_x = std::max(0, (int)x - blast_distance);
  const int max_x =
      std::min((int)x + blast_distance, (int)contest.arena().width() - 1);
  const int min_y = std::max(0, (int)y - blast_distance);
  const int max_y =
      std::min((int)y + blast_distance, (int)contest.arena().height() - 1);

  bool stop = false;

  const arena& arena = contest.arena();

  const auto check_cell = [&](int x, int y)
    {
      if (arena.is_static_wall(x, y) || !m_visibility_map(x, y))
        stop = true;
      else
        stop = visit(x, y) || m_solid_map(x, y);
    };

  for (int t = x - 1; !stop && (t >= min_x); --t)
    check_cell(t, y);

  stop = false;
  for (int t = x + 1; !stop && (t <= max_x); ++t)
    check_cell(t, y);

  stop = false;
  for (int t = y - 1; !stop && (t >= min_y); --t)
    check_cell(x, t);

  stop = false;
  for (int t = y + 1; !stop && (t <= max_y); ++t)
    check_cell(x, t);
}
