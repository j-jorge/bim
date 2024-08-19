// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>
#include <vector>

namespace bim::game
{
  class arena
  {
  public:
    arena();
    arena(std::uint8_t width, std::uint8_t height);

    std::uint8_t width() const;
    std::uint8_t height() const;

    entt::entity entity_at(std::uint8_t x, std::uint8_t y) const;
    void put_entity(std::uint8_t x, std::uint8_t y, entt::entity e);
    void erase_entity(std::uint8_t x, std::uint8_t y);

    bool is_solid(std::uint8_t x, std::uint8_t y) const;
    void set_solid(std::uint8_t x, std::uint8_t y);

    bool is_static_wall(std::uint8_t x, std::uint8_t y) const;
    void set_static_wall(std::uint8_t x, std::uint8_t y);

  private:
    std::uint8_t m_width;
    std::uint8_t m_height;

    std::vector<entt::entity> m_entities;
    std::vector<bool> m_walls;
    std::vector<bool> m_solids;
  };
}
