// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/table_2d.hpp>

#include <entt/entity/fwd.hpp>

#include <cstdint>
#include <vector>

namespace bim::game
{
  class arena;
  class entity_world_map;
  struct position_on_grid;

  class navigation_check
  {
  public:
    /**
     * Distance for solid cells at the border of the reachable area (i.e. we
     * could go on this cell if it was free.
     */
    static constexpr std::uint8_t border = 255;

    // Everything from this distance and above in unreachable.
    static constexpr std::uint8_t unreachable = 254;

  public:
    navigation_check();
    ~navigation_check();

    bool reachable(const arena& arena, std::uint8_t from_x,
                   std::uint8_t from_y, std::uint8_t to_x, std::uint8_t to_y);
    void paths(bim::table_2d<std::uint8_t>& distance,
               bim::table_2d<position_on_grid>& previous,
               const entt::registry& registry, const arena& arena,
               const entity_world_map& entity_map, std::uint8_t from_x,
               std::uint8_t from_y, const bim::table_2d<bool>& allowed);
    bool exists(const entt::registry& registry, const arena& arena,
                const entity_world_map& entity_map, std::uint8_t from_x,
                std::uint8_t from_y, int max_distance,
                const bim::table_2d<bool>& forbidden);

  private:
    enum class scan_loop_policy;

    template <scan_loop_policy ScanLoopPolicy, typename Enter, typename Visit,
              typename Distance>
    void scan(const arena& arena, std::uint8_t from_x, std::uint8_t from_y,
              Enter&& enter, Visit&& visit, Distance&& distance);

  private:
    bim::table_2d<bool> m_queued;
    std::vector<position_on_grid> m_pending;
  };
}
