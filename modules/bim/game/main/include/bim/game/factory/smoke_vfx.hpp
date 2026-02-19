// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <entt/entity/fwd.hpp>

#include <chrono>
#include <cstdint>

namespace bim::game
{
  class context;

  entt::entity
  in_out_smoke_vfx_factory(const context& context, entt::registry& registry,
                           std::uint8_t x, std::uint8_t y,
                           std::chrono::milliseconds initial_time);
}
