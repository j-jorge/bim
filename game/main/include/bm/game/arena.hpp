#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>
#include <vector>

namespace bm
{
  namespace game
  {
    class arena
    {
    public:
      arena(std::uint8_t width, std::uint8_t height);

      std::uint8_t width() const;
      std::uint8_t height() const;

      entt::entity entity_at(std::uint8_t x, std::uint8_t y) const;

      bool is_static_wall(std::uint8_t x, std::uint8_t y) const;
      void set_static_wall(std::uint8_t x, std::uint8_t y);

    private:
      std::uint8_t m_width;
      std::uint8_t m_height;

      std::vector<entt::entity> m_entities;
      std::vector<bool> m_walls;
    };
  }
}
