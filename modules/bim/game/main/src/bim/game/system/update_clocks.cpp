// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_clocks.hpp>

#include <bim/game/component/clock.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_clocks(entt::registry& registry,
                              std::chrono::milliseconds elapsed_time)
{
  registry.view<clock>().each(
      [elapsed_time](clock& c)
        {
          c.date += elapsed_time;
        });
}
