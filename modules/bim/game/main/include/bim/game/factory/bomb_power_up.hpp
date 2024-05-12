#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bim::game
{
  class arena;

  entt::entity bomb_power_up_factory(entt::registry& registry, arena& arena,
                                     std::uint8_t x, std::uint8_t y);
}
