// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/animator.hpp>

#include <bim/game/animation/animation_catalog.hpp>
#include <bim/game/animation/animation_specifications.hpp>
#include <bim/game/component/animation_state.hpp>
#include <bim/game/context/context.hpp>

#include <entt/entity/registry.hpp>

void bim::game::animator(const context& context, entt::registry& registry,
                         std::chrono::milliseconds elapsed_time)
{
  const animation_catalog& animations = context.get<const animation_catalog>();

  registry.view<animation_state>().each(
      [&registry, &animations, elapsed_time](entt::entity e,
                                             animation_state& s) -> void
      {
        const animation_specifications* m = &animations.get_animation(s.model);
        std::chrono::milliseconds new_time = s.elapsed_time + elapsed_time;

        while ((m->duration != std::chrono::milliseconds{})
               && (s.elapsed_time < m->duration) && (new_time >= m->duration))
          {
            if (m->dispatch_completion)
              m->dispatch_completion(registry, e);

            if (!m->next)
              break;

            s.model = m->next;
            s.elapsed_time = {};
            new_time -= m->duration;
            m = &animations.get_animation(m->next);
          }

        s.elapsed_time = new_time;
      });
}
