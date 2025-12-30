// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/update_falling_blocks.hpp>

#include <bim/game/component/crushed.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/falling_block.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/entity_world_map.hpp>
#include <bim/game/factory/wall.hpp>

#include <entt/entity/registry.hpp>

static void seal_falling_block(const bim::game::context& context,
                               entt::registry& registry,
                               bim::game::entity_world_map& entity_map,
                               entt::entity e,
                               const bim::game::position_on_grid& position)
{
  registry.emplace_or_replace<bim::game::dead>(e);

  for (const entt::entity entity_in_arena :
       entity_map.entities_at(position.x, position.y))
    registry.emplace_or_replace<bim::game::crushed>(entity_in_arena);

  entity_map.erase_entities(position.x, position.y);

  wall_factory(registry, entity_map, position.x, position.y);
}

void bim::game::update_falling_blocks(const context& context,
                                      entt::registry& registry,
                                      bim::game::entity_world_map& entity_map)
{
  registry.view<timer, falling_block, position_on_grid>().each(
      [&](entt::entity e, const timer& t,
          const position_on_grid& position) -> void
      {
        if (t.duration.count() > 0)
          return;

        seal_falling_block(context, registry, entity_map, e, position);
      });
}
