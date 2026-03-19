// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/feature_flags_fwd.hpp>
#include <bim/game/navigation_check.hpp>
#include <bim/game/random_generator.hpp>

#include <entt/fwd.hpp>

#include <array>
#include <cstdint>
#include <optional>

namespace bim::game
{
  class contest;
  struct fractional_position_on_grid;
  struct player;
  struct player_action;
  struct player_action_queue;

  class bot
  {
  public:
    bot(std::uint8_t player_index, std::uint8_t arena_width,
        std::uint8_t arena_height, std::uint64_t seed);
    ~bot();

    std::uint8_t player_index() const;

    player_action think(const contest& contest);

  private:
    struct goal
    {
      goal(int x, int y);

      position_on_grid target;
      bool drop_bomb;

      std::uint8_t crate_count;
      bool power_up;
      bool opponent_in_danger;
    };

    template <typename T>
    using per_player_array = std::array<T, g_max_player_count>;

  private:
    void find_players(
        const contest& contest, per_player_array<bool>& opponent_is_valid,
        per_player_array<position_on_grid>& player_positions,
        const player*& bot_player, const player_action_queue*& queued_actions,
        fractional_position_on_grid& bot_fractional_position) const;

    int build_visibility_map(const contest& contest);
    int build_crate_map(const contest& contest);
    void build_solid_map(const contest& contest);
    void build_danger_map(const contest& contest, const player& bot_player,
                          const player_action_queue& queued_actions);
    void build_power_up_map(const contest& contest);

    /// Simulate the explosion of the bomb.
    void store_bomb_danger(bim::table_2d<bool>& map, const contest& contest,
                           const position_on_grid& p, int d);

    bool goal_is_feasible(const contest& contest,
                          const player& bot_player) const;
    bool find_goal(const contest& contest,
                   const per_player_array<bool>& opponent_is_valid,
                   const per_player_array<position_on_grid>& player_positions,
                   const player& bot_player, int crate_count,
                   int unknown_count);
    bool find_safety_goal(const contest& contest,
                          const position_on_grid& bot_position);
    bool find_exploration_goal(
        const contest& contest,
        const per_player_array<bool>& opponent_is_valid,
        const per_player_array<position_on_grid>& player_positions,
        const player& bot_player);
    bool find_attack_goal(
        const contest& contest,
        const per_player_array<bool>& opponent_is_valid,
        const per_player_array<position_on_grid>& player_positions,
        const player& bot_player);

    goal
    analyze_cell(int& reward, const contest& contest,
                 const per_player_array<bool>& opponent_is_valid,
                 const per_player_array<position_on_grid>& player_positions,
                 const player& bot_player, bool bot_has_shield,
                 bool bot_in_open_area, std::size_t x, std::size_t y) const;
    void store_goal_path();

    player_action action_toward_goal(
        const contest& contest, const player& bot_player,
        const player_action_queue& queued_actions,
        const per_player_array<bool>& opponent_is_valid,
        const per_player_array<position_on_grid>& player_positions,
        const fractional_position_on_grid& bot_fractional_position);

    bool is_a_good_place_to_drop_a_bomb(
        const contest& contest, const player& bot_player,
        const player_action_queue& queued_actions,
        const per_player_array<bool>& opponent_is_valid,
        const per_player_array<position_on_grid>& player_positions,
        const fractional_position_on_grid& bot_fractional_position);

    bool simulate_bomb_drop(
        const contest& contest, const player& bot_player,
        const player_action_queue& queued_actions,
        const fractional_position_on_grid& bot_fractional_position);

    void
    flush_bot_actions(const contest& contest,
                      const player_action_queue& queued_actions,
                      fractional_position_on_grid& bot_fractional_position);

    void evaluate_bomb_explosion(
        const contest& contest, int blast_distance, int x, int y,
        const per_player_array<bool>& opponent_is_valid,
        const per_player_array<position_on_grid>& player_positions,
        int& crate_count, int& burning_power_up_count,
        int& burning_power_up_risk_count, int& opponents_in_danger) const;

    template <typename Visit>
    void simulate_bomb_explosion(const contest& contest, int blast_distance,
                                 int x, int y, Visit&& visit) const;

  private:
    std::optional<goal> m_goal;
    std::vector<position_on_grid> m_goal_path;
    std::size_t m_goal_step;

    bim::table_2d<bool> m_visibility_map;
    bim::table_2d<bool> m_crate_map;
    bim::table_2d<bool> m_solid_map;
    bim::table_2d<bool> m_immediate_danger_map;

    /**
     * This includes the flames but also the blast of the bombs that are going
     * to explode, as well as the burning crates if any.
     */
    bim::table_2d<bool> m_danger_map;
    bim::table_2d<entt::id_type> m_power_up_map;
    bim::table_2d<std::uint8_t> m_distance;
    bim::table_2d<position_on_grid> m_previous_cell;
    navigation_check m_navigation;

    std::vector<goal> m_candidate_goals;
    random_generator m_random;

    bool m_reach_for_safety;
    const std::uint8_t m_player_index;
  };
}
