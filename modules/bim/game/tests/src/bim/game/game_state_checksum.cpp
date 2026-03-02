// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/game_state_checksum.hpp>

#include <bim/game/component/bomb.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/flame_direction.hpp>
#include <bim/game/component/invisibility_state.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/game_state_serialization.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(bim_game_game_state_checksum, compare_serialization_checksum)
{
  entt::registry registry;
  const entt::entity entities[] = { registry.create(), registry.create(),
                                    registry.create(), registry.create() };

  registry.emplace<bim::game::bomb>(entities[0], 18);
  registry.emplace<bim::game::timer>(entities[0],
                                     std::chrono::milliseconds(2));

  registry.emplace<bim::game::invisibility_state>(entities[1], entities[3]);

  registry.emplace<bim::game::position_on_grid>(entities[1], 11, 22);
  registry.emplace<bim::game::player>(entities[1], 24, 0, 0, 4);

  registry.emplace<bim::game::flame>(entities[3],
                                     bim::game::flame_direction::left,
                                     bim::game::flame_segment::tip);
  registry.emplace<bim::game::dead>(entities[2]);

  bim::game::archive_storage archive;
  bim::game::serialize_state(archive, registry);

  bim::game::archive_storage final_state;
  {
    // Directly update the registry and compute the checksum. This would be the
    // behavior on the server.
    const entt::entity e = registry.create();
    registry.emplace<bim::game::bomb>(e, 53);
    registry.destroy(entities[1]);
  }
  const std::uint32_t checksum_a = bim::game::game_state_checksum(registry);

  {
    // Deserialize a previous state then redo the same actions before computing
    // the checksum. This would be the behavior on the clients.
    bim::game::deserialize_state(registry, archive);

    const entt::entity e = registry.create();
    registry.emplace<bim::game::bomb>(e, 53);
    registry.destroy(entities[1]);
  }

  const std::uint32_t checksum_b = bim::game::game_state_checksum(registry);

  EXPECT_EQ(checksum_a, checksum_b);
}
