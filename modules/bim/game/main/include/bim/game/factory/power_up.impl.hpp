// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/factory/power_up.hpp>

#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/entity_world_map.hpp>

#include <entt/entity/registry.hpp>

namespace bim::game
{
  template <typename PowerUpComponent>
  entt::entity power_up_factory(entt::registry& registry,
                                entity_world_map& entity_map, std::uint8_t x,
                                std::uint8_t y)
  {
    const entt::entity entity = registry.create();

    registry.emplace<PowerUpComponent>(entity);
    registry.emplace<position_on_grid>(entity, x, y);

    entity_map.put_entity(entity, x, y);

    return entity;
  }
}
