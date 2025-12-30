// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/power_up.impl.hpp>

#include <bim/game/component/bomb_power_up.hpp>
#include <bim/game/component/flame_power_up.hpp>
#include <bim/game/component/invisibility_power_up.hpp>
#include <bim/game/component/shield_power_up.hpp>
#include <bim/game/entity_world_map.hpp>

template entt::entity bim::game::power_up_factory<bim::game::bomb_power_up>(
    entt::registry& registry, entity_world_map& entity_map, std::uint8_t x,
    std::uint8_t y);

template entt::entity bim::game::power_up_factory<bim::game::flame_power_up>(
    entt::registry& registry, entity_world_map& entity_map, std::uint8_t x,
    std::uint8_t y);

template entt::entity
bim::game::power_up_factory<bim::game::invisibility_power_up>(
    entt::registry& registry, entity_world_map& entity_map, std::uint8_t x,
    std::uint8_t y);

template entt::entity bim::game::power_up_factory<bim::game::shield_power_up>(
    entt::registry& registry, entity_world_map& entity_map, std::uint8_t x,
    std::uint8_t y);
