// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/player_death_marker.hpp>

#include <bim/game/component/player_death_marker.hpp>

#include <entt/entity/registry.hpp>

entt::entity
bim::game::player_death_marker_factory(entt::registry& registry,
                                       std::uint8_t player_index,
                                       player_death_kind death_kind)
{
  const entt::entity entity = registry.create();

  registry.emplace<player_death_marker>(entity, player_index, death_kind);

  return entity;
}
