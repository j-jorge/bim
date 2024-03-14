#include <bim/game/system/remove_dead_objects.hpp>

#include <bim/game/component/dead.hpp>

#include <entt/entity/registry.hpp>

void bim::game::remove_dead_objects(entt::registry& registry)
{
  registry.view<dead>().each(
      [&](entt::entity e) -> void
      {
        registry.destroy(e);
      });
}
