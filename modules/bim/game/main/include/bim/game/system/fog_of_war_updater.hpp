// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/cell_neighborhood_fwd.hpp>
#include <bim/game/constant/max_player_count.hpp>

#include <bim/table_2d.hpp>

#include <entt/entity/fwd.hpp>

#include <chrono>

namespace bim::game
{
  class arena;
  struct fog_of_war;
  struct timer;
  enum class fog_state;

  namespace detail
  {
    struct fog_properties
    {
      bim::table_2d<bim::game::timer*> timer;
      bim::table_2d<fog_of_war*> fog;
    };
  }

  class fog_of_war_updater
  {
  public:
    fog_of_war_updater(entt::registry& registry, const arena& arena,
                       std::uint8_t player_count);
    ~fog_of_war_updater();

    const bim::table_2d<fog_of_war*>& fog(std::size_t player_index) const;

    void update(entt::registry& registry);

  private:
    void update_fog(entt::registry& registry, std::size_t player_index,
                    int player_x, int player_y);
    void build_maps(entt::registry& registry);

    void uncover_around_player(const detail::fog_properties& p, int player_x,
                               int player_y) const;
    void uncover_around_flames(entt::registry& registry,
                               const detail::fog_properties& p);

    void update_opacity_from_timers(entt::registry& registry,
                                    const detail::fog_properties& p) const;

  private:
    bim::table_2d<bool> m_blown;
    std::array<detail::fog_properties, g_max_player_count> m_tables;

    const std::uint8_t m_player_count;
  };
}
