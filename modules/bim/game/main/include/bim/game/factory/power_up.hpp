// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <cstdint>

namespace bim::game
{
  class arena;

  template <typename PowerUpComponent>
  entt::entity power_up_factory(entt::registry& registry, arena& arena,
                                std::uint8_t x, std::uint8_t y);
}
