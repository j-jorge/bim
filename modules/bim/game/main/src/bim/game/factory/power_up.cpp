// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/factory/power_up.impl.hpp>

namespace bim::game
{
  struct bomb_power_up;
  struct flame_power_up;
  struct invisibility_power_up;

  template entt::entity
  power_up_factory<bomb_power_up>(entt::registry& registry, arena& arena,
                                  std::uint8_t x, std::uint8_t y);

  template entt::entity
  power_up_factory<flame_power_up>(entt::registry& registry, arena& arena,
                                   std::uint8_t x, std::uint8_t y);

  template entt::entity power_up_factory<invisibility_power_up>(
      entt::registry& registry, arena& arena, std::uint8_t x, std::uint8_t y);
}
