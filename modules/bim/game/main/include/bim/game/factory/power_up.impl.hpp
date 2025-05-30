// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/factory/power_up.hpp>

#include <bim/game/arena.hpp>

#include <bim/game/component/position_on_grid.hpp>

#include <entt/entity/registry.hpp>

namespace bim::game
{
  template <typename PowerUpComponent>
  entt::entity power_up_factory(entt::registry& registry, arena& arena,
                                std::uint8_t x, std::uint8_t y)
  {
    const entt::entity entity = registry.create();

    registry.emplace<PowerUpComponent>(entity);
    registry.emplace<position_on_grid>(entity, x, y);

    arena.put_entity(x, y, entity);

    return entity;
  }
}
