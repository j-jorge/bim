// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_power_up_spawners.impl.hpp>

#include <bim/game/component/bomb_power_up_spawner.hpp>
#include <bim/game/component/flame_power_up_spawner.hpp>
#include <bim/game/component/invisibility_power_up_spawner.hpp>
#include <bim/game/component/shield_power_up_spawner.hpp>

template void
bim::game::update_power_up_spawners<bim::game::bomb_power_up_spawner>(
    entt::registry& registry, arena& arena);

template void
bim::game::update_power_up_spawners<bim::game::flame_power_up_spawner>(
    entt::registry& registry, arena& arena);

template void
bim::game::update_power_up_spawners<bim::game::invisibility_power_up_spawner>(
    entt::registry& registry, arena& arena);

template void
bim::game::update_power_up_spawners<bim::game::shield_power_up_spawner>(
    entt::registry& registry, arena& arena);
