// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/component/player_death_kind_fwd.hpp>

#include <entt/entity/fwd.hpp>

namespace bim::game
{
  entt::entity player_death_marker_factory(entt::registry& registry,
                                           std::uint8_t player_index,
                                           player_death_kind death_kind);
}
