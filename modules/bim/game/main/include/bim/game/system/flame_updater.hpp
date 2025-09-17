// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/table_2d.hpp>

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  class arena;

  class flame_updater
  {
  public:
    flame_updater(std::uint8_t arena_width, std::uint8_t arena_height);
    ~flame_updater();

    void update(entt::registry& registry, arena& arena);

  private:
    bim::table_2d<bool> m_flame_map;
  };
}
