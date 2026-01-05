// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/remove_dead_objects.hpp>

#include <bim/game/component/crushed.hpp>
#include <bim/game/component/dead.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/entity_world_map.hpp>
#include <bim/game/factory/invincibility_state.hpp>
#include <bim/game/factory/invisibility_state.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

template <typename T>
class remove_dead_objects_test : public testing::Test
{
public:
  remove_dead_objects_test();

protected:
  entt::registry m_registry;
  bim::game::entity_world_map m_entity_map;
  const entt::entity m_alive;
  const entt::entity m_dead;
};

template <typename T>
remove_dead_objects_test<T>::remove_dead_objects_test()
  : m_entity_map(3, 3)
  , m_alive(m_registry.create())
  , m_dead(m_registry.create())
{
  m_registry.emplace<T>(m_dead);
}

using death_causes = testing::Types<bim::game::crushed, bim::game::dead>;

TYPED_TEST_SUITE(remove_dead_objects_test, death_causes);

TYPED_TEST(remove_dead_objects_test, simple_entity)
{
  bim::game::remove_dead_objects(this->m_registry, this->m_entity_map);

  EXPECT_TRUE(this->m_registry.valid(this->m_alive));
  EXPECT_FALSE(this->m_registry.valid(this->m_dead));
}

TYPED_TEST(remove_dead_objects_test, invisibility)
{
  const entt::entity alive_invisible = bim::game::invisibility_state_factory(
      this->m_registry, this->m_alive, std::chrono::seconds(1));
  const entt::entity dead_invisible = bim::game::invisibility_state_factory(
      this->m_registry, this->m_dead, std::chrono::seconds(1));

  bim::game::remove_dead_objects(this->m_registry, this->m_entity_map);

  EXPECT_TRUE(this->m_registry.valid(this->m_alive));
  EXPECT_TRUE(this->m_registry.valid(alive_invisible));
  EXPECT_FALSE(this->m_registry.valid(this->m_dead));
  EXPECT_FALSE(this->m_registry.valid(dead_invisible));
}

TYPED_TEST(remove_dead_objects_test, invincibility)
{
  const entt::entity alive_invisible = bim::game::invincibility_state_factory(
      this->m_registry, this->m_alive, std::chrono::seconds(1));
  const entt::entity dead_invisible = bim::game::invincibility_state_factory(
      this->m_registry, this->m_dead, std::chrono::seconds(1));

  bim::game::remove_dead_objects(this->m_registry, this->m_entity_map);

  EXPECT_TRUE(this->m_registry.valid(this->m_alive));
  EXPECT_TRUE(this->m_registry.valid(alive_invisible));
  EXPECT_FALSE(this->m_registry.valid(this->m_dead));
  EXPECT_FALSE(this->m_registry.valid(dead_invisible));
}

TYPED_TEST(remove_dead_objects_test, position_on_grid)
{
  const bim::game::position_on_grid alive_position =
      this->m_registry.template emplace<bim::game::position_on_grid>(
          this->m_alive, 2, 1);
  this->m_entity_map.put_entity(this->m_alive, alive_position.x,
                                alive_position.y);

  const bim::game::position_on_grid dead_position =
      this->m_registry.template emplace<bim::game::position_on_grid>(
          this->m_dead, 1, 2);
  this->m_entity_map.put_entity(this->m_dead, dead_position.x,
                                dead_position.y);

  bim::game::remove_dead_objects(this->m_registry, this->m_entity_map);

  EXPECT_TRUE(this->m_registry.valid(this->m_alive));
  ASSERT_EQ(1,
            this->m_entity_map.entities_at(alive_position.x, alive_position.y)
                .size());
  EXPECT_EQ(this->m_alive, this->m_entity_map.entities_at(
                               alive_position.x, alive_position.y)[0]);

  EXPECT_FALSE(this->m_registry.valid(this->m_dead));
  EXPECT_TRUE(this->m_entity_map.entities_at(dead_position.x, dead_position.y)
                  .empty());
}

TYPED_TEST(remove_dead_objects_test, fractional_position_on_grid)
{
  const bim::game::fractional_position_on_grid alive_position =
      this->m_registry
          .template emplace<bim::game::fractional_position_on_grid>(
              this->m_alive, 2.5, 1.5);
  this->m_entity_map.put_entity(this->m_alive, alive_position.grid_aligned_x(),
                                alive_position.grid_aligned_y());

  const bim::game::fractional_position_on_grid dead_position =
      this->m_registry
          .template emplace<bim::game::fractional_position_on_grid>(
              this->m_dead, 1.5, 2.5);
  this->m_entity_map.put_entity(this->m_dead, dead_position.grid_aligned_x(),
                                dead_position.grid_aligned_y());

  bim::game::remove_dead_objects(this->m_registry, this->m_entity_map);

  EXPECT_TRUE(this->m_registry.valid(this->m_alive));
  ASSERT_EQ(1, this->m_entity_map
                   .entities_at(alive_position.grid_aligned_x(),
                                alive_position.grid_aligned_y())
                   .size());
  EXPECT_EQ(this->m_alive, this->m_entity_map.entities_at(
                               alive_position.grid_aligned_x(),
                               alive_position.grid_aligned_y())[0]);

  EXPECT_FALSE(this->m_registry.valid(this->m_dead));
  EXPECT_TRUE(this->m_entity_map
                  .entities_at(dead_position.grid_aligned_x(),
                               dead_position.grid_aligned_y())
                  .empty());
}
