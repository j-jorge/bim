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
      arena(entt::registry& registry, std::uint8_t width, std::uint8_t height);

      std::uint8_t width() const;
      std::uint8_t height() const;

      entt::entity at(uint8_t x, uint8_t y) const;

    private:
      std::uint8_t m_width;
      std::uint8_t m_height;

      std::vector<entt::entity> m_cells;
    };
  }
}
