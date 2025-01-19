// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/apply_player_action.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/cell_neighborhood.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/system/refresh_bomb_inventory.hpp>
#include <bim/game/system/update_bombs.hpp>
#include <bim/game/system/update_timers.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

class bim_game_apply_player_action_test : public ::testing::Test
{
public:
  bim_game_apply_player_action_test();

protected:
  void run_forward_move_test(int start_x, int start_y,
                             bim::game::player_movement movement, float end_x,
                             float end_y);
  void run_dodge_move_test(int start_x, int start_y,
                           bim::game::player_movement first_move,
                           bim::game::player_movement second_move, float end_x,
                           float end_y);

protected:
  entt::registry m_registry;
  bim::game::arena m_arena;
};

bim_game_apply_player_action_test::bim_game_apply_player_action_test()
  : m_arena(5, 5)
{ /*
   Set-up the walls like this:

   XXXXX
   X   X
   X X X
   X   X
   XXXXX
 */

  for (int x = 0; x != m_arena.width(); ++x)
    m_arena.set_static_wall(x, 0, bim::game::cell_neighborhood::none);

  m_arena.set_static_wall(0, 1, bim::game::cell_neighborhood::none);
  m_arena.set_static_wall(m_arena.width() - 1, 1,
                          bim::game::cell_neighborhood::none);

  m_arena.set_static_wall(0, 2, bim::game::cell_neighborhood::none);
  m_arena.set_static_wall(2, 2, bim::game::cell_neighborhood::none);
  m_arena.set_static_wall(m_arena.width() - 1, 2,
                          bim::game::cell_neighborhood::none);

  m_arena.set_static_wall(0, 3, bim::game::cell_neighborhood::none);
  m_arena.set_static_wall(m_arena.width() - 1, 3,
                          bim::game::cell_neighborhood::none);

  for (int x = 0; x != m_arena.width(); ++x)
    m_arena.set_static_wall(x, 4, bim::game::cell_neighborhood::none);
}

void bim_game_apply_player_action_test::run_forward_move_test(
    int start_x, int start_y, bim::game::player_movement movement, float end_x,
    float end_y)
{
  const entt::entity player =
      bim::game::player_factory(m_registry, 0, start_x, start_y);
  const bim::game::fractional_position_on_grid& position =
      m_registry.storage<bim::game::fractional_position_on_grid>().get(player);

  EXPECT_FLOAT_EQ(start_x + 0.5f, position.x_float());
  EXPECT_FLOAT_EQ(start_y + 0.5f, position.y_float());

  bim::game::player_action& action =
      m_registry.storage<bim::game::player_action>().get(player);

  for (int i = 0; i != bim::game::g_player_steps_per_cell; ++i)
    {
      action.movement = movement;
      bim::game::apply_player_action(m_registry, m_arena);
    }

  // Flush the queue.
  action = {};
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(m_registry, m_arena);

  EXPECT_FLOAT_EQ(end_x, position.x_float());
  EXPECT_FLOAT_EQ(end_y, position.y_float());
}

void bim_game_apply_player_action_test::run_dodge_move_test(
    int start_x, int start_y, bim::game::player_movement first_move,
    bim::game::player_movement second_move, float end_x, float end_y)
{
  const entt::entity player =
      bim::game::player_factory(m_registry, 0, start_x, start_y);
  const bim::game::fractional_position_on_grid& position =
      m_registry.storage<bim::game::fractional_position_on_grid>().get(player);

  EXPECT_FLOAT_EQ(start_x + 0.5f, position.x_float());
  EXPECT_FLOAT_EQ(start_y + 0.5f, position.y_float());

  bim::game::player_action& action =
      m_registry.storage<bim::game::player_action>().get(player);

  for (int i = 0; i != bim::game::g_player_steps_per_cell / 2 - 1; ++i)
    {
      action.movement = first_move;
      bim::game::apply_player_action(m_registry, m_arena);
    }

  for (int i = 0; i != bim::game::g_player_steps_per_cell; ++i)
    {
      action.movement = second_move;
      bim::game::apply_player_action(m_registry, m_arena);
    }

  // Flush the queue.
  action = {};
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(m_registry, m_arena);

  EXPECT_FLOAT_EQ(end_x, position.x_float());
  EXPECT_FLOAT_EQ(end_y, position.y_float());
}

TEST_F(bim_game_apply_player_action_test, move_right_freely)
{
  run_forward_move_test(1, 1, bim::game::player_movement::right, 2.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, move_left_freely)
{
  run_forward_move_test(3, 1, bim::game::player_movement::left, 2.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, move_down_freely)
{
  run_forward_move_test(1, 1, bim::game::player_movement::down, 1.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_up_freely)
{
  run_forward_move_test(1, 3, bim::game::player_movement::up, 1.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_right)
{
  run_forward_move_test(3, 1, bim::game::player_movement::right, 3.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_left)
{
  run_forward_move_test(1, 1, bim::game::player_movement::left, 1.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_down)
{
  run_forward_move_test(1, 3, bim::game::player_movement::down, 1.5, 3.5);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_up)
{
  run_forward_move_test(1, 1, bim::game::player_movement::up, 1.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, move_right_up_around_wall)
{
  run_dodge_move_test(1, 1, bim::game::player_movement::down,
                      bim::game::player_movement::right, 2.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, move_right_down_around_wall)
{
  run_dodge_move_test(1, 3, bim::game::player_movement::up,
                      bim::game::player_movement::right, 2.5, 3.5);
}

TEST_F(bim_game_apply_player_action_test, move_left_up_around_wall)
{
  run_dodge_move_test(3, 1, bim::game::player_movement::down,
                      bim::game::player_movement::left, 2.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, move_left_down_around_wall)
{
  run_dodge_move_test(3, 3, bim::game::player_movement::up,
                      bim::game::player_movement::left, 2.5, 3.5);
}

TEST_F(bim_game_apply_player_action_test, move_down_left_around_wall)
{
  run_dodge_move_test(1, 1, bim::game::player_movement::right,
                      bim::game::player_movement::down, 1.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_down_right_around_wall)
{
  run_dodge_move_test(3, 1, bim::game::player_movement::left,
                      bim::game::player_movement::down, 3.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_up_left_around_wall)
{
  run_dodge_move_test(1, 3, bim::game::player_movement::right,
                      bim::game::player_movement::up, 1.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_up_right_around_wall)
{
  run_dodge_move_test(3, 3, bim::game::player_movement::left,
                      bim::game::player_movement::up, 3.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, cannot_go_through_bombs)
{
  constexpr int player_count = 2;
  constexpr int start_x[player_count] = { 1, 1 };
  constexpr int start_y[player_count] = { 1, 3 };
  const entt::entity players[player_count] = {
    bim::game::player_factory(m_registry, 0, start_x[0], start_y[0]),
    bim::game::player_factory(m_registry, 1, start_x[1], start_y[1])
  };

  bim::game::player_action* const action[player_count] = {
    &m_registry.storage<bim::game::player_action>().get(players[0]),
    &m_registry.storage<bim::game::player_action>().get(players[1])
  };

  action[1]->drop_bomb = true;

  for (int i = 0; i != 2 * bim::game::g_player_steps_per_cell; ++i)
    {
      action[0]->movement = bim::game::player_movement::down;
      action[1]->movement = bim::game::player_movement::right;
      bim::game::apply_player_action(m_registry, m_arena);
    }

  // Flush the queue.
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(m_registry, m_arena);

  const bim::game::fractional_position_on_grid positions[player_count] = {
    m_registry.storage<bim::game::fractional_position_on_grid>().get(
        players[0]),
    m_registry.storage<bim::game::fractional_position_on_grid>().get(
        players[1])
  };

  EXPECT_FLOAT_EQ(start_x[0] + 0.5f, positions[0].x_float());
  EXPECT_FLOAT_EQ(start_y[0] + 1.5f, positions[0].y_float());

  EXPECT_EQ(start_x[0], positions[0].grid_aligned_x());
  EXPECT_EQ(start_y[0] + 1, positions[0].grid_aligned_y());

  EXPECT_EQ(start_x[1] + 2, positions[1].grid_aligned_x());
  EXPECT_EQ(start_y[1], positions[1].grid_aligned_y());
}

TEST_F(bim_game_apply_player_action_test, drop_bomb_decrease_inventory)
{
  constexpr int player_index = 3;
  constexpr int start_x = 1;
  constexpr int start_y = 1;
  const entt::entity player_entity =
      bim::game::player_factory(m_registry, player_index, start_x, start_y);

  bim::game::player& player =
      m_registry.storage<bim::game::player>().get(player_entity);
  bim::game::fractional_position_on_grid& position =
      m_registry.storage<bim::game::fractional_position_on_grid>().get(
          player_entity);

  player.bomb_capacity = 2;
  player.bomb_available = player.bomb_capacity;

  bim::game::player_action& action =
      m_registry.storage<bim::game::player_action>().get(player_entity);

  // One bomb
  action.drop_bomb = true;

  // Flush the queue.
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(m_registry, m_arena);

  EXPECT_EQ(1, player.bomb_available);

  const entt::entity bomb_entity =
      m_arena.entity_at(position.grid_aligned_x(), position.grid_aligned_y());
  bim::game::bomb bomb =
      m_registry.storage<bim::game::bomb>().get(bomb_entity);
  EXPECT_EQ(player_index, bomb.player_index);
  position.y += 1;

  // Second bomb
  action.drop_bomb = true;

  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(m_registry, m_arena);

  EXPECT_EQ(0, player.bomb_available);

  bomb = m_registry.storage<bim::game::bomb>().get(bomb_entity);
  EXPECT_EQ(player_index, bomb.player_index);

  position.y += 1;

  // Third bomb, not dropped as the player has no bomb left.
  action.drop_bomb = true;

  for (int i = 0; i != bim::game::player_action_queue::queue_size; ++i)
    bim::game::apply_player_action(m_registry, m_arena);

  EXPECT_EQ(0, player.bomb_available);

  EXPECT_TRUE(entt::null
              == m_arena.entity_at(position.grid_aligned_x(),
                                   position.grid_aligned_y()));

  bim::game::timer timer =
      m_registry.storage<bim::game::timer>().get(bomb_entity);
  // Explode the bombs. The player should get his inventory back.
  bim::game::update_timers(m_registry, timer.duration);
  bim::game::update_bombs(m_registry, m_arena);
  bim::game::refresh_bomb_inventory(m_registry);
  EXPECT_EQ(2, player.bomb_available);
}

TEST_F(bim_game_apply_player_action_test, drop_bomb_where_the_player_was)
{
  constexpr int player_index = 0;
  constexpr int start_x = 1;
  constexpr int start_y = 1;
  const entt::entity player_entity =
      bim::game::player_factory(m_registry, player_index, start_x, start_y);

  bim::game::player& player =
      m_registry.storage<bim::game::player>().get(player_entity);

  bim::game::fractional_position_on_grid& position =
      m_registry.storage<bim::game::fractional_position_on_grid>().get(
          player_entity);

  // Move the player down, at the limit where it is still located in the
  // initial cell.
  using coordinate_type = bim::game::fractional_position_on_grid::value_type;
  for (coordinate_type e(1); e != coordinate_type(0); e /= 2)
    {
      bim::game::fractional_position_on_grid p = position;
      p.y += e;

      if (p.grid_aligned_y() == position.grid_aligned_y())
        position = p;
    }

  EXPECT_EQ(start_x, position.grid_aligned_x());
  EXPECT_EQ(start_y, position.grid_aligned_y());

  bim::game::player_action& action =
      m_registry.storage<bim::game::player_action>().get(player_entity);

  player.bomb_capacity = 1;
  player.bomb_available = player.bomb_capacity;

  // Fill the queue with downward movements.
  for (int i = 0; i != bim::game::player_action_queue::queue_size; ++i)
    {
      action.movement = bim::game::player_movement::down;
      bim::game::apply_player_action(m_registry, m_arena);
    }

  // The player has not moved, this is where the bomb should be dropped.
  EXPECT_EQ(start_x, position.grid_aligned_x());
  EXPECT_EQ(start_y, position.grid_aligned_y());

  // Queue the dropping.
  action.drop_bomb = true;
  action.movement = bim::game::player_movement::idle;
  bim::game::apply_player_action(m_registry, m_arena);

  action.drop_bomb = false;

  // Then flush the queue.
  for (int i = 0; i != bim::game::player_action_queue::queue_size; ++i)
    bim::game::apply_player_action(m_registry, m_arena);

  // The bomb has been dropped.
  EXPECT_EQ(0, player.bomb_available);

  const entt::entity bomb_entity = m_arena.entity_at(start_x, start_y);
  ASSERT_TRUE(bomb_entity != entt::null);

  EXPECT_TRUE(m_registry.storage<bim::game::bomb>().contains(bomb_entity));
}

TEST_F(bim_game_apply_player_action_test, drop_bomb_flood)
{
  constexpr int player_index = 0;
  constexpr int start_x = 1;
  constexpr int start_y = 1;
  const entt::entity player_entity =
      bim::game::player_factory(m_registry, player_index, start_x, start_y);

  bim::game::player& player =
      m_registry.storage<bim::game::player>().get(player_entity);

  player.bomb_capacity = 1;
  player.bomb_available = player.bomb_capacity;

  bim::game::player_action& action =
      m_registry.storage<bim::game::player_action>().get(player_entity);

  // Keep dropping bombs at each iteration while moving down.
  for (bim::game::fractional_position_on_grid& position =
           m_registry.storage<bim::game::fractional_position_on_grid>().get(
               player_entity);
       position.grid_aligned_y() != start_y + 2;)
    {
      action.drop_bomb = true;
      action.movement = bim::game::player_movement::down;
      bim::game::apply_player_action(m_registry, m_arena);
    }

  // Drop a bomb in the last cell too.
  action.drop_bomb = true;
  action.movement = bim::game::player_movement::down;
  bim::game::apply_player_action(m_registry, m_arena);

  // Flush the queue.
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(m_registry, m_arena);

  EXPECT_EQ(0, player.bomb_available);

  // There should be a bomb in the first cell and none in the others.
  const entt::entity bomb_entity = m_arena.entity_at(start_x, start_y);
  ASSERT_TRUE(bomb_entity != entt::null);

  EXPECT_TRUE(m_registry.storage<bim::game::bomb>().contains(bomb_entity));

  EXPECT_TRUE(m_arena.entity_at(start_x, start_y + 1) == entt::null);
  EXPECT_TRUE(m_arena.entity_at(start_x, start_y + 2) == entt::null);
}
