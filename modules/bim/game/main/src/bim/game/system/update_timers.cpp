// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_timers.hpp>

#include <bim/game/component/crushed.hpp>
#include <bim/game/component/timer.hpp>

#include <entt/entity/registry.hpp>

void bim::game::update_timers(entt::registry& registry,
                              std::chrono::milliseconds elapsed_time)
{
  registry.view<timer>().each(
      [elapsed_time](timer& t)
      {
        if (t.duration <= elapsed_time)
          t.duration = {};
        else
          t.duration -= elapsed_time;
      });
}

void bim::game::trigger_crushed_timers(entt::registry& registry)
{
  registry.view<timer, crushed>().each(
      [](timer& t)
      {
        t.duration = {};
      });
}
