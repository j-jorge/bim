// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/apply_player_action.hpp>

#include <bim/game/arena.hpp>
#include <bim/game/cell_edge.hpp>
#include <bim/game/cell_neighborhood.hpp>
#include <bim/game/component/animation_state.hpp>
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/burning.hpp>
#include <bim/game/component/flame.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/component/timer.hpp>
#include <bim/game/context/context.hpp>
#include <bim/game/context/fill_context.hpp>
#include <bim/game/context/player_animations.hpp>
#include <bim/game/entity_world_map.hpp>
#include <bim/game/factory/bomb.hpp>
#include <bim/game/factory/crate.hpp>
#include <bim/game/factory/player.hpp>
#include <bim/game/system/animator.hpp>
#include <bim/game/system/refresh_bomb_inventory.hpp>
#include <bim/game/system/remove_dead_objects.hpp>
#include <bim/game/system/update_bombs.hpp>
#include <bim/game/system/update_crates.hpp>
#include <bim/game/system/update_flames.hpp>
#include <bim/game/system/update_timers.hpp>

#include <entt/entity/registry.hpp>

#include <cstdio>

#include <gtest/gtest.h>

class bim_game_apply_player_action_test : public ::testing::Test
{
public:
  bim_game_apply_player_action_test();

protected:
  enum class obstacle
  {
    bomb,
    crate,
    fence,
    static_wall
  };

protected:
  static const char* obstacle_name(obstacle o);

  void create_obstacle(int x, int y, obstacle o, bim::game::cell_edge edges);
  void move_halfway(bim::game::player_action& action,
                    bim::game::player_movement movement);
  void flush_actions();

  void run_all_forward_move_tests(bim::game::cell_edge edges, int start_x,
                                  int start_y,
                                  bim::game::player_movement movement,
                                  bim::game::animation_id end_state,
                                  float end_x, float end_y);
  void run_forward_move_test(int start_x, int start_y,
                             bim::game::player_movement movement,
                             bim::game::animation_id end_state, float end_x,
                             float end_y);

  void run_all_dodge_move_tests(bim::game::cell_edge edges, int start_x,
                                int start_y,
                                bim::game::player_movement first_move,
                                bim::game::player_movement second_move,
                                bim::game::animation_id end_state, float end_x,
                                float end_y);
  void run_dodge_move_test(int start_x, int start_y,
                           bim::game::player_movement first_move,
                           bim::game::player_movement second_move,
                           bim::game::animation_id end_state, float end_x,
                           float end_y);
  void run_all_cannot_dodge_tests(int start_x, int start_y, int front_dx,
                                  int front_dy, int side_dx, int side_dy,
                                  bim::game::player_movement forward_move,
                                  bim::game::player_movement side_move,
                                  bim::game::animation_id end_state,
                                  bim::game::cell_edge front_edge,
                                  bim::game::cell_edge side_edge,
                                  bim::game::cell_edge side_front_edge);

protected:
  entt::registry m_registry;
  bim::game::context m_context;
  bim::game::player_animations m_player_animations;
  bim::game::arena m_arena;
  bim::game::entity_world_map m_entity_map;
};

bim_game_apply_player_action_test::bim_game_apply_player_action_test()
  : m_arena(5, 5)
  , m_entity_map(m_arena.width(), m_arena.height())
{
  bim::game::fill_context(m_context);
  m_player_animations = m_context.get<const bim::game::player_animations>();

  /*
   Set-up the walls like this:

   XXXXX
   X   X
   X   X
   X   X
   XXXXX
 */

  for (int x = 0; x != m_arena.width(); ++x)
    m_arena.set_static_wall(x, 0, bim::game::cell_neighborhood::none);

  m_arena.set_static_wall(0, 1, bim::game::cell_neighborhood::none);
  m_arena.set_static_wall(m_arena.width() - 1, 1,
                          bim::game::cell_neighborhood::none);

  m_arena.set_static_wall(0, 2, bim::game::cell_neighborhood::none);
  m_arena.set_static_wall(m_arena.width() - 1, 2,
                          bim::game::cell_neighborhood::none);

  m_arena.set_static_wall(0, 3, bim::game::cell_neighborhood::none);
  m_arena.set_static_wall(m_arena.width() - 1, 3,
                          bim::game::cell_neighborhood::none);

  for (int x = 0; x != m_arena.width(); ++x)
    m_arena.set_static_wall(x, 4, bim::game::cell_neighborhood::none);
}

void bim_game_apply_player_action_test::create_obstacle(
    int x, int y, obstacle o, bim::game::cell_edge edges)
{
  switch (o)
    {
    case obstacle::bomb:
      bim::game::bomb_factory(m_registry, m_entity_map, x, y, 1, 0,
                              std::chrono::milliseconds(0));
      break;
    case obstacle::crate:
      bim::game::crate_factory(m_registry, m_entity_map, x, y);
      break;
    case obstacle::fence:
      if (edges != bim::game::cell_edge::none)
        m_arena.add_fence(x, y, edges);
      break;
    case obstacle::static_wall:
      m_arena.set_static_wall(x, y, bim::game::cell_neighborhood::none);
      break;
    }
}

void bim_game_apply_player_action_test::move_halfway(
    bim::game::player_action& action, bim::game::player_movement movement)
{
  for (int i = 0; i != bim::game::g_player_steps_per_cell / 2 - 1; ++i)
    {
      action.movement = movement;
      bim::game::apply_player_action(m_context, m_registry, m_arena,
                                     m_entity_map);
    }
}

void bim_game_apply_player_action_test::flush_actions()
{
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(m_context, m_registry, m_arena,
                                   m_entity_map);
}

const char* bim_game_apply_player_action_test::obstacle_name(obstacle o)
{
  switch (o)
    {
    case obstacle::bomb:
      return "bomb";
    case obstacle::crate:
      return "crate";
    case obstacle::fence:
      return "fence";
    case obstacle::static_wall:
      return "static_wall";
    }
  return "-";
}

void bim_game_apply_player_action_test::run_all_forward_move_tests(
    bim::game::cell_edge edges, int start_x, int start_y,
    bim::game::player_movement movement, bim::game::animation_id end_state,
    float end_x, float end_y)
{
  bim::game::arena arena = m_arena;
  bim::game::entity_world_map entity_map = m_entity_map;

  for (obstacle o : { obstacle::bomb, obstacle::crate, obstacle::fence,
                      obstacle::static_wall })
    {
      m_registry.clear();
      m_arena = arena;
      m_entity_map = entity_map;

      create_obstacle(2, 2, o, edges);
      run_forward_move_test(start_x, start_y, movement, end_state, end_x,
                            end_y);

      if (HasFailure())
        std::printf("Obstacle was '%s'.\n", obstacle_name(o));
    }
}

void bim_game_apply_player_action_test::run_forward_move_test(
    int start_x, int start_y, bim::game::player_movement movement,
    bim::game::animation_id end_state, float end_x, float end_y)
{
  const entt::entity player =
      bim::game::player_factory(m_registry, m_entity_map, 0, start_x, start_y,
                                m_player_animations.idle_down);
  const bim::game::fractional_position_on_grid& position =
      m_registry.storage<bim::game::fractional_position_on_grid>().get(player);

  EXPECT_FLOAT_EQ(start_x + 0.5f, position.x_float());
  EXPECT_FLOAT_EQ(start_y + 0.5f, position.y_float());

  bim::game::player_action& action =
      m_registry.storage<bim::game::player_action>().get(player);

  for (int i = 0; i != bim::game::g_player_steps_per_cell; ++i)
    {
      action.movement = movement;
      bim::game::apply_player_action(m_context, m_registry, m_arena,
                                     m_entity_map);
    }

  bim::game::animation_state& state =
      m_registry.storage<bim::game::animation_state>().get(player);
  EXPECT_EQ(end_state, state.model);

  flush_actions();

  EXPECT_FLOAT_EQ(end_x, position.x_float());
  EXPECT_FLOAT_EQ(end_y, position.y_float());
}

void bim_game_apply_player_action_test::run_all_dodge_move_tests(
    bim::game::cell_edge edges, int start_x, int start_y,
    bim::game::player_movement first_move,
    bim::game::player_movement second_move, bim::game::animation_id end_state,
    float end_x, float end_y)
{
  bim::game::arena arena = m_arena;
  bim::game::entity_world_map entity_map = m_entity_map;

  for (obstacle o : { obstacle::bomb, obstacle::crate, obstacle::fence,
                      obstacle::static_wall })
    {
      m_registry.clear();
      m_arena = arena;
      m_entity_map = entity_map;

      create_obstacle(2, 2, o, edges);
      run_dodge_move_test(start_x, start_y, first_move, second_move, end_state,
                          end_x, end_y);

      if (HasFailure())
        std::printf("Obstacle was '%s'.\n", obstacle_name(o));
    }
}

void bim_game_apply_player_action_test::run_dodge_move_test(
    int start_x, int start_y, bim::game::player_movement first_move,
    bim::game::player_movement second_move, bim::game::animation_id end_state,
    float end_x, float end_y)
{
  const entt::entity player =
      bim::game::player_factory(m_registry, m_entity_map, 0, start_x, start_y,
                                m_player_animations.idle_down);
  const bim::game::fractional_position_on_grid& position =
      m_registry.storage<bim::game::fractional_position_on_grid>().get(player);

  EXPECT_FLOAT_EQ(start_x + 0.5f, position.x_float());
  EXPECT_FLOAT_EQ(start_y + 0.5f, position.y_float());

  bim::game::player_action& action =
      m_registry.storage<bim::game::player_action>().get(player);

  move_halfway(action, first_move);

  for (int i = 0; i != bim::game::g_player_steps_per_cell; ++i)
    {
      action.movement = second_move;
      bim::game::apply_player_action(m_context, m_registry, m_arena,
                                     m_entity_map);
    }

  bim::game::animation_state& state =
      m_registry.storage<bim::game::animation_state>().get(player);
  EXPECT_EQ(end_state, state.model);

  flush_actions();

  EXPECT_FLOAT_EQ(end_x, position.x_float());
  EXPECT_FLOAT_EQ(end_y, position.y_float());
}

void bim_game_apply_player_action_test::run_all_cannot_dodge_tests(
    int start_x, int start_y, int front_dx, int front_dy, int side_dx,
    int side_dy, bim::game::player_movement forward_move,
    bim::game::player_movement side_move, bim::game::animation_id end_state,
    bim::game::cell_edge front_edge, bim::game::cell_edge side_edge,
    bim::game::cell_edge side_front_edge)
{
  const obstacle all_obstacles[] = { obstacle::bomb, obstacle::crate,
                                     obstacle::fence, obstacle::static_wall };
  const obstacle side_obstacles[] = { obstacle::bomb,  obstacle::crate,
                                      obstacle::fence, obstacle::fence,
                                      obstacle::fence, obstacle::static_wall };
  const bim::game::cell_edge side_edges[] = {
    {}, {}, {}, side_edge, side_front_edge, {}
  };
  static_assert(std::size(side_edges) == std::size(side_obstacles));

  bim::game::arena arena = m_arena;
  bim::game::entity_world_map entity_map = m_entity_map;

  for (obstacle front : all_obstacles)
    for (std::size_t i = 0; i != std::size(side_edges); ++i)
      for (obstacle corner : all_obstacles)
        {
          m_registry.clear();
          m_arena = arena;
          m_entity_map = entity_map;

          create_obstacle(start_x + front_dx, start_y + front_dy, front,
                          front_edge);
          create_obstacle(start_x + side_dx, start_y + side_dy,
                          side_obstacles[i], side_edges[i]);
          create_obstacle(start_x + (front_dx != 0 ? front_dx : side_dx),
                          start_y + (front_dy != 0 ? front_dy : side_dy),
                          corner, front_edge);

          const entt::entity player = bim::game::player_factory(
              m_registry, m_entity_map, 0, start_x, start_y,
              m_player_animations.idle_down);
          const bim::game::fractional_position_on_grid& position =
              m_registry.storage<bim::game::fractional_position_on_grid>().get(
                  player);

          EXPECT_FLOAT_EQ(start_x + 0.5f, position.x_float());
          EXPECT_FLOAT_EQ(start_y + 0.5f, position.y_float());

          bim::game::player_action& action =
              m_registry.storage<bim::game::player_action>().get(player);

          action.movement = forward_move;
          bim::game::apply_player_action(m_context, m_registry, m_arena,
                                         m_entity_map);

          flush_actions();

          // The player should not have moved.
          EXPECT_FLOAT_EQ(start_x + 0.5f, position.x_float());
          EXPECT_FLOAT_EQ(start_y + 0.5f, position.y_float());

          {
            const bim::game::animation_state& state =
                m_registry.storage<bim::game::animation_state>().get(player);
            EXPECT_EQ(end_state, state.model);
          }

          // Move the player a bit on the side
          move_halfway(action, side_move);
          flush_actions();

          // If we cannot move to the side, we don't try to.
          const bool can_move_on_the_side =
              (side_edges[i] == bim::game::cell_edge::none)
              || (side_edges[i] != side_edge);

          const float ref_x =
              can_move_on_the_side ? position.x_float() : (start_x + 0.5f);
          const float ref_y =
              can_move_on_the_side ? position.y_float() : (start_y + 0.5f);

          // Move the player a bit on the side
          action.movement = forward_move;
          bim::game::apply_player_action(m_context, m_registry, m_arena,
                                         m_entity_map);

          flush_actions();

          // The player should not have moved.
          EXPECT_FLOAT_EQ(ref_x, position.x_float());
          EXPECT_FLOAT_EQ(ref_y, position.y_float());

          {
            const bim::game::animation_state& state =
                m_registry.storage<bim::game::animation_state>().get(player);
            EXPECT_EQ(end_state, state.model);
          }

          if (HasFailure())
            {
              std::printf("front=%s, side[%d]=(%s,0x%x), corner=%s, "
                          "can_move_on_the_side=%d\n",
                          obstacle_name(front), (int)i,
                          obstacle_name(side_obstacles[i]), (int)side_edges[i],
                          obstacle_name(corner), can_move_on_the_side);
              return;
            }
        }
}

TEST_F(bim_game_apply_player_action_test, move_right_freely)
{
  run_all_forward_move_tests(bim::game::cell_edge::all, 1, 1,
                             bim::game::player_movement::right,
                             m_player_animations.walk_right, 2.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, move_left_freely)
{
  run_all_forward_move_tests(bim::game::cell_edge::all, 3, 1,
                             bim::game::player_movement::left,
                             m_player_animations.walk_left, 2.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, move_down_freely)
{
  run_all_forward_move_tests(bim::game::cell_edge::all, 1, 1,
                             bim::game::player_movement::down,
                             m_player_animations.walk_down, 1.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_up_freely)
{
  run_all_forward_move_tests(bim::game::cell_edge::all, 1, 3,
                             bim::game::player_movement::up,
                             m_player_animations.walk_up, 1.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_right)
{
  run_all_forward_move_tests(bim::game::cell_edge::left, 1, 2,
                             bim::game::player_movement::right,
                             m_player_animations.idle_right, 1.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_left)
{
  run_all_forward_move_tests(bim::game::cell_edge::right, 3, 2,
                             bim::game::player_movement::left,
                             m_player_animations.idle_left, 3.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_down)
{
  run_all_forward_move_tests(bim::game::cell_edge::up, 2, 1,
                             bim::game::player_movement::down,
                             m_player_animations.idle_down, 2.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_up)
{
  run_all_forward_move_tests(bim::game::cell_edge::down, 2, 3,
                             bim::game::player_movement::up,
                             m_player_animations.idle_up, 2.5, 3.5);
}

TEST_F(bim_game_apply_player_action_test, move_right_up_around_wall)
{
  /*
      ┌──>
      │╔═╗
    ──┘║ ║
       ╚═╝
  */
  run_all_dodge_move_tests(bim::game::cell_edge::left, 1, 1,
                           bim::game::player_movement::down,
                           bim::game::player_movement::right,
                           m_player_animations.walk_right, 2.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, move_right_down_around_wall)
{
  /*
       ╔═╗
    ──┐║ ║
      │╚═╝
      └──>
  */
  run_all_dodge_move_tests(bim::game::cell_edge::left, 1, 3,
                           bim::game::player_movement::up,
                           bim::game::player_movement::right,
                           m_player_animations.walk_right, 2.5, 3.5);
}

TEST_F(bim_game_apply_player_action_test, move_left_up_around_wall)
{
  /*
    <──┐
    ╔═╗│
    ║ ║└──
    ╚═╝
  */
  run_all_dodge_move_tests(bim::game::cell_edge::right, 3, 1,
                           bim::game::player_movement::down,
                           bim::game::player_movement::left,
                           m_player_animations.walk_left, 2.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, move_left_down_around_wall)
{
  /*
    ╔═╗
    ║ ║┌──
    ╚═╝│
    <──┘
  */
  run_all_dodge_move_tests(bim::game::cell_edge::right, 3, 3,
                           bim::game::player_movement::up,
                           bim::game::player_movement::left,
                           m_player_animations.walk_left, 2.5, 3.5);
}

TEST_F(bim_game_apply_player_action_test, move_down_left_around_wall)
{
  /*
     │
   ┌─┘
   │╔═╗
   v║ ║
    ╚═╝
  */
  run_all_dodge_move_tests(bim::game::cell_edge::up, 1, 1,
                           bim::game::player_movement::right,
                           bim::game::player_movement::down,
                           m_player_animations.walk_down, 1.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_down_right_around_wall)
{
  /*
     │
     └─┐
    ╔═╗│
    ║ ║v
    ╚═╝
  */
  run_all_dodge_move_tests(bim::game::cell_edge::up, 3, 1,
                           bim::game::player_movement::left,
                           bim::game::player_movement::down,
                           m_player_animations.walk_down, 3.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_up_left_around_wall)
{
  /*
    ╔═╗
   ^║ ║
   │╚═╝
   └─┐
     │
  */
  run_all_dodge_move_tests(
      bim::game::cell_edge::down, 1, 3, bim::game::player_movement::right,
      bim::game::player_movement::up, m_player_animations.walk_up, 1.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_up_right_around_wall)
{
  /*
   ╔═╗
   ║ ║^
   ╚═╝│
    ┌─┘
    │
  */
  run_all_dodge_move_tests(
      bim::game::cell_edge::down, 3, 3, bim::game::player_movement::left,
      bim::game::player_movement::up, m_player_animations.walk_up, 3.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_right_straighten_up)
{
  /*
     ┌──>
   ──┘
      ╔═╗
      ║ ║
      ╚═╝
  */
  run_all_dodge_move_tests(bim::game::cell_edge::left, 1, 1,
                           bim::game::player_movement::down,
                           bim::game::player_movement::right,
                           m_player_animations.walk_right, 2.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, move_right_straighten_down)
{
  /*
      ╔═╗
      ║ ║
      ╚═╝
   ──┐
     └──>
  */
  run_all_dodge_move_tests(bim::game::cell_edge::left, 1, 3,
                           bim::game::player_movement::up,
                           bim::game::player_movement::right,
                           m_player_animations.walk_right, 2.5, 3.5);
}

TEST_F(bim_game_apply_player_action_test, move_left_straighten_up)
{
  /*
      <──┐
         └──
      ╔═╗
      ║ ║
      ╚═╝
  */
  run_all_dodge_move_tests(bim::game::cell_edge::right, 3, 1,
                           bim::game::player_movement::down,
                           bim::game::player_movement::left,
                           m_player_animations.walk_left, 2.5, 1.5);
}

TEST_F(bim_game_apply_player_action_test, move_left_straighten_down)
{
  /*
      ╔═╗
      ║ ║
      ╚═╝
         ┌──
      <──┘
  */
  run_all_dodge_move_tests(bim::game::cell_edge::right, 3, 3,
                           bim::game::player_movement::up,
                           bim::game::player_movement::left,
                           m_player_animations.walk_left, 2.5, 3.5);
}

TEST_F(bim_game_apply_player_action_test, move_down_straighten_left)
{
  /*
      │
     ┌┘
     │ ╔═╗
     v ║ ║
       ╚═╝
  */
  run_all_dodge_move_tests(bim::game::cell_edge::up, 1, 1,
                           bim::game::player_movement::right,
                           bim::game::player_movement::down,
                           m_player_animations.walk_down, 1.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_down_straighten_right)
{
  /*
       │
       └┐
    ╔═╗ │
    ║ ║ v
    ╚═╝
  */
  run_all_dodge_move_tests(bim::game::cell_edge::up, 3, 1,
                           bim::game::player_movement::left,
                           bim::game::player_movement::down,
                           m_player_animations.walk_down, 3.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_up_straighten_left)
{
  /*
       ╔═╗
     ^ ║ ║
     │ ╚═╝
     └┐
      │
  */
  run_all_dodge_move_tests(
      bim::game::cell_edge::down, 1, 3, bim::game::player_movement::right,
      bim::game::player_movement::up, m_player_animations.walk_up, 1.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, move_up_straighten_right)
{
  /*
    ╔═╗
    ║ ║ ^
    ╚═╝ │
       ┌┘
       │
  */
  run_all_dodge_move_tests(
      bim::game::cell_edge::down, 3, 3, bim::game::player_movement::left,
      bim::game::player_movement::up, m_player_animations.walk_up, 3.5, 2.5);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_right_up_around_wall)
{
  run_all_cannot_dodge_tests(
      1, 2, 1, 0, 0, -1, bim::game::player_movement::right,
      bim::game::player_movement::up, m_player_animations.idle_right,
      bim::game::cell_edge::left, bim::game::cell_edge::down,
      bim::game::cell_edge::right);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_right_down_around_wall)
{
  run_all_cannot_dodge_tests(
      1, 2, 1, 0, 0, 1, bim::game::player_movement::right,
      bim::game::player_movement::down, m_player_animations.idle_right,
      bim::game::cell_edge::left, bim::game::cell_edge::up,
      bim::game::cell_edge::right);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_left_up_around_wall)
{
  run_all_cannot_dodge_tests(
      3, 2, -1, 0, 0, -1, bim::game::player_movement::left,
      bim::game::player_movement::up, m_player_animations.idle_left,
      bim::game::cell_edge::right, bim::game::cell_edge::down,
      bim::game::cell_edge::left);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_left_down_around_wall)
{
  run_all_cannot_dodge_tests(
      3, 2, -1, 0, 0, 1, bim::game::player_movement::left,
      bim::game::player_movement::down, m_player_animations.idle_left,
      bim::game::cell_edge::right, bim::game::cell_edge::up,
      bim::game::cell_edge::left);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_down_left_around_wall)
{
  run_all_cannot_dodge_tests(
      2, 1, 0, 1, -1, 0, bim::game::player_movement::down,
      bim::game::player_movement::left, m_player_animations.idle_down,
      bim::game::cell_edge::up, bim::game::cell_edge::right,
      bim::game::cell_edge::down);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_down_right_around_wall)
{
  run_all_cannot_dodge_tests(
      2, 1, 0, 1, 1, 0, bim::game::player_movement::down,
      bim::game::player_movement::right, m_player_animations.idle_down,
      bim::game::cell_edge::up, bim::game::cell_edge::left,
      bim::game::cell_edge::down);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_up_left_around_wall)
{
  run_all_cannot_dodge_tests(
      2, 3, 0, -1, -1, 0, bim::game::player_movement::up,
      bim::game::player_movement::left, m_player_animations.idle_up,
      bim::game::cell_edge::down, bim::game::cell_edge::right,
      bim::game::cell_edge::up);
}

TEST_F(bim_game_apply_player_action_test, cannot_move_up_right_around_wall)
{
  run_all_cannot_dodge_tests(
      2, 3, 0, -1, 1, 0, bim::game::player_movement::up,
      bim::game::player_movement::right, m_player_animations.idle_up,
      bim::game::cell_edge::down, bim::game::cell_edge::left,
      bim::game::cell_edge::up);
}

TEST_F(bim_game_apply_player_action_test, drop_bomb_decreases_inventory)
{
  constexpr int player_index = 3;
  constexpr int start_x = 1;
  constexpr int start_y = 1;
  const entt::entity player_entity = bim::game::player_factory(
      m_registry, m_entity_map, player_index, start_x, start_y,
      m_player_animations.idle_down);

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

  flush_actions();

  EXPECT_EQ(1, player.bomb_available);

  const std::span<const entt::entity> entities = m_entity_map.entities_at(
      position.grid_aligned_x(), position.grid_aligned_y());
  ASSERT_EQ(2, entities.size());

  const bim::game::bomb* bomb = nullptr;
  entt::entity bomb_entity = entt::null;

  for (entt::entity e : entities)
    if (m_registry.storage<bim::game::bomb>().contains(e))
      {
        bomb_entity = e;
        bomb = &m_registry.storage<bim::game::bomb>().get(e);
        break;
      }

  ASSERT_FALSE(bomb_entity == entt::null);
  ASSERT_NE(nullptr, bomb);

  EXPECT_EQ(player_index, bomb->player_index);
  position.y += 1;

  // Second bomb
  action.drop_bomb = true;

  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(m_context, m_registry, m_arena,
                                   m_entity_map);

  EXPECT_EQ(0, player.bomb_available);

  ASSERT_TRUE(m_registry.storage<bim::game::bomb>().contains(bomb_entity));

  bomb = &m_registry.storage<bim::game::bomb>().get(bomb_entity);
  EXPECT_EQ(player_index, bomb->player_index);

  position.y += 1;

  // Third bomb, not dropped as the player has no bomb left.
  action.drop_bomb = true;

  for (int i = 0; i != bim::game::player_action_queue::queue_size; ++i)
    bim::game::apply_player_action(m_context, m_registry, m_arena,
                                   m_entity_map);

  EXPECT_EQ(0, player.bomb_available);

  EXPECT_TRUE(
      m_entity_map
          .entities_at(position.grid_aligned_x(), position.grid_aligned_y())
          .empty());

  bim::game::timer timer =
      m_registry.storage<bim::game::timer>().get(bomb_entity);
  // Explode the bombs. The player should get his inventory back.
  bim::game::update_timers(m_registry, timer.duration);
  bim::game::update_bombs(m_context, m_registry, m_arena, m_entity_map);
  bim::game::remove_dead_objects(m_registry, m_entity_map);
  bim::game::refresh_bomb_inventory(m_registry);
  EXPECT_EQ(2, player.bomb_available);
}

TEST_F(bim_game_apply_player_action_test, drop_bomb_where_the_player_was)
{
  constexpr int player_index = 0;
  constexpr int start_x = 1;
  constexpr int start_y = 1;
  const entt::entity player_entity = bim::game::player_factory(
      m_registry, m_entity_map, player_index, start_x, start_y,
      m_player_animations.idle_down);

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
      bim::game::apply_player_action(m_context, m_registry, m_arena,
                                     m_entity_map);
    }

  // The player has not moved, this is where the bomb should be dropped.
  EXPECT_EQ(start_x, position.grid_aligned_x());
  EXPECT_EQ(start_y, position.grid_aligned_y());

  // Queue the dropping.
  action.drop_bomb = true;
  action.movement = bim::game::player_movement::idle;
  bim::game::apply_player_action(m_context, m_registry, m_arena, m_entity_map);

  action.drop_bomb = false;

  // Then flush the queue.
  flush_actions();

  // The bomb has been dropped.
  EXPECT_EQ(0, player.bomb_available);

  const std::span<const entt::entity> entities =
      m_entity_map.entities_at(start_x, start_y);
  ASSERT_FALSE(entities.empty());

  const entt::entity bomb_entity = entities[0];

  EXPECT_TRUE(m_registry.storage<bim::game::bomb>().contains(bomb_entity));
}

TEST_F(bim_game_apply_player_action_test, drop_bomb_flood)
{
  constexpr int player_index = 0;
  constexpr int start_x = 1;
  constexpr int start_y = 1;
  const entt::entity player_entity = bim::game::player_factory(
      m_registry, m_entity_map, player_index, start_x, start_y,
      m_player_animations.idle_down);

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
      bim::game::apply_player_action(m_context, m_registry, m_arena,
                                     m_entity_map);
    }

  // Drop a bomb in the last cell too.
  action.drop_bomb = true;
  action.movement = bim::game::player_movement::down;
  bim::game::apply_player_action(m_context, m_registry, m_arena, m_entity_map);

  // Flush the queue.
  flush_actions();

  EXPECT_EQ(0, player.bomb_available);

  // There should be a bomb in the first cell and none in the others.
  const std::span<const entt::entity> entities =
      m_entity_map.entities_at(start_x, start_y);
  ASSERT_FALSE(entities.empty());

  const entt::entity bomb_entity = entities[0];

  EXPECT_TRUE(m_registry.storage<bim::game::bomb>().contains(bomb_entity));

  EXPECT_TRUE(m_entity_map.entities_at(start_x, start_y + 1).empty());
  ASSERT_EQ(1, m_entity_map.entities_at(start_x, start_y + 2).size());
  EXPECT_EQ(player_entity, m_entity_map.entities_at(start_x, start_y + 2)[0]);
}

TEST_F(bim_game_apply_player_action_test, cannot_drop_bomb_on_existing_bomb)
{
  constexpr int start_x = 1;
  constexpr int start_y = 1;
  constexpr int player_count = 4;
  const entt::entity player_entity[player_count] = {
    bim::game::player_factory(m_registry, m_entity_map, 0, start_x, start_y,
                              m_player_animations.idle_down),
    bim::game::player_factory(m_registry, m_entity_map, 1, start_x, start_y,
                              m_player_animations.idle_down),
    bim::game::player_factory(m_registry, m_entity_map, 2, start_x, start_y,
                              m_player_animations.idle_down),
    bim::game::player_factory(m_registry, m_entity_map, 3, start_x, start_y,
                              m_player_animations.idle_down)
  };

  bim::game::player* const player[player_count] = {
    &m_registry.storage<bim::game::player>().get(player_entity[0]),
    &m_registry.storage<bim::game::player>().get(player_entity[1]),
    &m_registry.storage<bim::game::player>().get(player_entity[2]),
    &m_registry.storage<bim::game::player>().get(player_entity[3])
  };

  constexpr int bomb_capacity = 2;

  for (int i = 0; i != player_count; ++i)
    {
      player[i]->bomb_capacity = bomb_capacity;
      player[i]->bomb_available = bomb_capacity;
    }

  bim::game::player_action* const action[player_count] = {
    &m_registry.storage<bim::game::player_action>().get(player_entity[0]),
    &m_registry.storage<bim::game::player_action>().get(player_entity[1]),
    &m_registry.storage<bim::game::player_action>().get(player_entity[2]),
    &m_registry.storage<bim::game::player_action>().get(player_entity[3])
  };

  // One bomb
  for (int i = 0; i != player_count; ++i)
    action[i]->drop_bomb = true;

  // Flush the queue.
  flush_actions();

  int player_index = -1;
  for (int i = 0; i != player_count; ++i)
    if (player[i]->bomb_available == 1)
      {
        player_index = i;
        break;
      }

  ASSERT_NE(-1, player_index);

  for (int i = 0; i != player_count; ++i)
    if (i != player_index)
      {
        EXPECT_EQ(bomb_capacity, player[i]->bomb_available) << "i=" << i;
      }

  const std::span<const entt::entity> entities =
      m_entity_map.entities_at(start_x, start_y);
  ASSERT_EQ(5, entities.size());

  for (int i = 0; i != player_count; ++i)
    EXPECT_NE(entities.end(), std::ranges::find(entities, player_entity[i]))
        << "i=" << i;

  const bim::game::bomb* bomb = nullptr;
  for (entt::entity e : entities)
    if (m_registry.storage<bim::game::bomb>().contains(e))
      {
        bomb = &m_registry.storage<bim::game::bomb>().get(e);
        break;
      }

  ASSERT_NE(nullptr, bomb);
  EXPECT_EQ(player_index, bomb->player_index);
}

TEST(bim_game_apply_player_action, cannot_walk_through_bombs)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::arena arena(5, 5);
  bim::game::entity_world_map entity_map(arena.width(), arena.height());
  constexpr std::uint8_t x = 2;
  constexpr std::uint8_t y = 2;
  constexpr std::uint8_t strength = 1;
  constexpr std::uint8_t player_index = 0;

  constexpr int player_count = 4;
  const std::uint8_t start_x[player_count] = { 2, 3, 2, 1 };
  const std::uint8_t start_y[player_count] = { 1, 2, 3, 2 };

  /*
    Initial state:

    .....
    ..0..
    .3ó1.
    ..2..
    .....
  */
  bim::game::bomb_factory(registry, entity_map, x, y, strength, player_index,
                          std::chrono::milliseconds(0));

  const bim::game::player_animations& player_animations =
      context.get<const bim::game::player_animations>();
  const entt::entity players[player_count] = {
    bim::game::player_factory(registry, entity_map, 0, start_x[0], start_y[0],
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 1, start_x[1], start_y[1],
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 2, start_x[2], start_y[2],
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 3, start_x[3], start_y[3],
                              player_animations.idle_down)
  };

  bim::game::player_action* const action[player_count] = {
    &registry.storage<bim::game::player_action>().get(players[0]),
    &registry.storage<bim::game::player_action>().get(players[1]),
    &registry.storage<bim::game::player_action>().get(players[2]),
    &registry.storage<bim::game::player_action>().get(players[3])
  };

  const bim::game::fractional_position_on_grid* const
      positions[player_count] = {
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[0]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[1]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[2]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[3])
      };

  // Move the players toward the bomb, they should keep their positions.
  for (int i = 0; i != bim::game::g_player_steps_per_cell; ++i)
    {
      action[0]->movement = bim::game::player_movement::down;
      action[1]->movement = bim::game::player_movement::left;
      action[2]->movement = bim::game::player_movement::up;
      action[3]->movement = bim::game::player_movement::right;
      bim::game::apply_player_action(context, registry, arena, entity_map);
    }

  // Flush the action queue.
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(context, registry, arena, entity_map);

  // The players should be at their initial positions.
  for (int i = 0; i != player_count; ++i)
    {
      EXPECT_FLOAT_EQ(start_x[i] + 0.5f, positions[i]->x_float()) << "i=" << i;
      EXPECT_FLOAT_EQ(start_y[i] + 0.5f, positions[i]->y_float()) << "i=" << i;
    }

  // Now move the players to the corners, such that they won't be burned by the
  // exploding bomb.
  for (int i = 0; i != bim::game::g_player_steps_per_cell; ++i)
    {
      action[0]->movement = bim::game::player_movement::left;
      action[1]->movement = bim::game::player_movement::up;
      action[2]->movement = bim::game::player_movement::right;
      action[3]->movement = bim::game::player_movement::down;
      bim::game::apply_player_action(context, registry, arena, entity_map);
    }

  // Flush the action queue.
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(context, registry, arena, entity_map);

  // Trigger the bomb.
  bim::game::update_timers(registry, std::chrono::milliseconds(12));
  bim::game::update_bombs(context, registry, arena, entity_map);

  // Update the flames until they disappear.
  for (int i = 0; i != 100; ++i)
    {
      constexpr std::chrono::milliseconds d(12);

      bim::game::update_timers(registry, d);
      bim::game::animator(context, registry, d);
      bim::game::update_flames(context, registry, entity_map);
      bim::game::remove_dead_objects(registry, entity_map);

      if (registry.storage<bim::game::flame>().empty())
        break;
    }

  ASSERT_TRUE(registry.storage<bim::game::flame>().empty());

  // Move the players back to their initial positions.
  for (int i = 0; i != bim::game::g_player_steps_per_cell; ++i)
    {
      action[0]->movement = bim::game::player_movement::right;
      action[1]->movement = bim::game::player_movement::down;
      action[2]->movement = bim::game::player_movement::left;
      action[3]->movement = bim::game::player_movement::up;
      bim::game::apply_player_action(context, registry, arena, entity_map);
    }

  // Then move the players where the bomb was.
  for (int i = 0; i != bim::game::g_player_steps_per_cell; ++i)
    {
      action[0]->movement = bim::game::player_movement::down;
      action[1]->movement = bim::game::player_movement::left;
      action[2]->movement = bim::game::player_movement::up;
      action[3]->movement = bim::game::player_movement::right;
      bim::game::apply_player_action(context, registry, arena, entity_map);
    }

  // Flush the action queue.
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(context, registry, arena, entity_map);

  for (int i = 0; i != player_count; ++i)
    {
      EXPECT_FLOAT_EQ(x + 0.5, positions[i]->x_float()) << "i=" << i;
      EXPECT_FLOAT_EQ(y + 0.5, positions[i]->y_float()) << "i=" << i;
    }
}

TEST(bim_game_apply_player_action, cannot_walk_through_crates)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::arena arena(5, 5);
  bim::game::entity_world_map entity_map(arena.width(), arena.height());
  constexpr std::uint8_t x = 2;
  constexpr std::uint8_t y = 2;

  constexpr int player_count = 4;
  const std::uint8_t start_x[player_count] = { 2, 3, 2, 1 };
  const std::uint8_t start_y[player_count] = { 1, 2, 3, 2 };

  /*
    Initial state:

    .....
    ..0..
    .3C1.
    ..2..
    .....
  */
  const entt::entity crate_entity =
      bim::game::crate_factory(registry, entity_map, x, y);

  const bim::game::player_animations& player_animations =
      context.get<const bim::game::player_animations>();
  const entt::entity players[player_count] = {
    bim::game::player_factory(registry, entity_map, 0, start_x[0], start_y[0],
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 1, start_x[1], start_y[1],
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 2, start_x[2], start_y[2],
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 3, start_x[3], start_y[3],
                              player_animations.idle_down)
  };

  bim::game::player_action* const action[player_count] = {
    &registry.storage<bim::game::player_action>().get(players[0]),
    &registry.storage<bim::game::player_action>().get(players[1]),
    &registry.storage<bim::game::player_action>().get(players[2]),
    &registry.storage<bim::game::player_action>().get(players[3])
  };

  const bim::game::fractional_position_on_grid* const
      positions[player_count] = {
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[0]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[1]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[2]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[3])
      };

  // Move the players toward the bomb, they should keep their positions.
  for (int i = 0; i != bim::game::g_player_steps_per_cell; ++i)
    {
      action[0]->movement = bim::game::player_movement::down;
      action[1]->movement = bim::game::player_movement::left;
      action[2]->movement = bim::game::player_movement::up;
      action[3]->movement = bim::game::player_movement::right;
      bim::game::apply_player_action(context, registry, arena, entity_map);
    }

  // Flush the action queue.
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(context, registry, arena, entity_map);

  // The players should be at their initial positions.
  for (int i = 0; i != player_count; ++i)
    {
      EXPECT_FLOAT_EQ(start_x[i] + 0.5f, positions[i]->x_float()) << "i=" << i;
      EXPECT_FLOAT_EQ(start_y[i] + 0.5f, positions[i]->y_float()) << "i=" << i;
    }

  // Now remove the crates.
  registry.emplace<bim::game::burning>(crate_entity);
  bim::game::update_crates(registry, entity_map);

  // Then move the players where the crate was.
  for (int i = 0; i != bim::game::g_player_steps_per_cell; ++i)
    {
      action[0]->movement = bim::game::player_movement::down;
      action[1]->movement = bim::game::player_movement::left;
      action[2]->movement = bim::game::player_movement::up;
      action[3]->movement = bim::game::player_movement::right;
      bim::game::apply_player_action(context, registry, arena, entity_map);
    }

  // Flush the action queue.
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(context, registry, arena, entity_map);

  for (int i = 0; i != player_count; ++i)
    {
      EXPECT_FLOAT_EQ(x + 0.5, positions[i]->x_float()) << "i=" << i;
      EXPECT_FLOAT_EQ(y + 0.5, positions[i]->y_float()) << "i=" << i;
    }
}

TEST(bim_game_apply_player_action, cannot_walk_through_fences_from_outside)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::arena arena(5, 5);
  bim::game::entity_world_map entity_map(arena.width(), arena.height());

  constexpr int player_count = 4;
  const std::uint8_t start_x = 2;
  const std::uint8_t start_y = 2;

  /*
    All players in the middle, with a fence on each side:

    .....
    ..┴..
    .┤P├.
    ..┬..
    .....
  */
  arena.add_fence(2, 1, bim::game::cell_edge::down);
  arena.add_fence(2, 3, bim::game::cell_edge::up);
  arena.add_fence(1, 2, bim::game::cell_edge::right);
  arena.add_fence(3, 2, bim::game::cell_edge::left);

  const bim::game::player_animations& player_animations =
      context.get<const bim::game::player_animations>();
  const entt::entity players[player_count] = {
    bim::game::player_factory(registry, entity_map, 0, start_x, start_y,
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 1, start_x, start_y,
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 2, start_x, start_y,
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 3, start_x, start_y,
                              player_animations.idle_down)
  };

  bim::game::player_action* const action[player_count] = {
    &registry.storage<bim::game::player_action>().get(players[0]),
    &registry.storage<bim::game::player_action>().get(players[1]),
    &registry.storage<bim::game::player_action>().get(players[2]),
    &registry.storage<bim::game::player_action>().get(players[3])
  };

  const bim::game::fractional_position_on_grid* const
      positions[player_count] = {
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[0]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[1]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[2]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[3])
      };

  // Move the players toward the fences, they should keep their positions.
  for (int i = 0; i != bim::game::g_player_steps_per_cell; ++i)
    {
      action[0]->movement = bim::game::player_movement::down;
      action[1]->movement = bim::game::player_movement::left;
      action[2]->movement = bim::game::player_movement::up;
      action[3]->movement = bim::game::player_movement::right;
      bim::game::apply_player_action(context, registry, arena, entity_map);
    }

  // Flush the action queue.
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(context, registry, arena, entity_map);

  // The players should be at their initial positions.
  for (int i = 0; i != player_count; ++i)
    {
      EXPECT_FLOAT_EQ(start_x + 0.5f, positions[i]->x_float()) << "i=" << i;
      EXPECT_FLOAT_EQ(start_y + 0.5f, positions[i]->y_float()) << "i=" << i;
    }
}

TEST(bim_game_apply_player_action, cannot_walk_through_fences_from_inside)
{
  bim::game::context context;
  bim::game::fill_context(context);

  entt::registry registry;
  bim::game::arena arena(5, 5);
  bim::game::entity_world_map entity_map(arena.width(), arena.height());

  constexpr int player_count = 4;
  const std::uint8_t start_x = 2;
  const std::uint8_t start_y = 2;

  /*
    All players in the middle, with a fence on the opposite edge of each
    adjacent cell:

    .....
    ..┬..
    .├P┤.
    ..┴.
    .....
  */
  arena.add_fence(2, 1, bim::game::cell_edge::up);
  arena.add_fence(2, 3, bim::game::cell_edge::down);
  arena.add_fence(1, 2, bim::game::cell_edge::left);
  arena.add_fence(3, 2, bim::game::cell_edge::right);

  const bim::game::player_animations& player_animations =
      context.get<const bim::game::player_animations>();
  const entt::entity players[player_count] = {
    bim::game::player_factory(registry, entity_map, 0, start_x, start_y,
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 1, start_x, start_y,
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 2, start_x, start_y,
                              player_animations.idle_down),
    bim::game::player_factory(registry, entity_map, 3, start_x, start_y,
                              player_animations.idle_down)
  };

  bim::game::player_action* const action[player_count] = {
    &registry.storage<bim::game::player_action>().get(players[0]),
    &registry.storage<bim::game::player_action>().get(players[1]),
    &registry.storage<bim::game::player_action>().get(players[2]),
    &registry.storage<bim::game::player_action>().get(players[3])
  };

  const bim::game::fractional_position_on_grid* const
      positions[player_count] = {
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[0]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[1]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[2]),
        &registry.storage<bim::game::fractional_position_on_grid>().get(
            players[3])
      };

  // Move the players toward the fences.
  for (int i = 0; i != 2 * bim::game::g_player_steps_per_cell; ++i)
    {
      action[0]->movement = bim::game::player_movement::down;
      action[1]->movement = bim::game::player_movement::left;
      action[2]->movement = bim::game::player_movement::up;
      action[3]->movement = bim::game::player_movement::right;
      bim::game::apply_player_action(context, registry, arena, entity_map);
    }

  // Flush the action queue.
  for (int i = 0; i != bim::game::player_action_queue::queue_size + 1; ++i)
    bim::game::apply_player_action(context, registry, arena, entity_map);

  // The players should be in the adjacent cells but should not have reached
  // the middle.

  // Player 0 moved one cell down.
  EXPECT_FLOAT_EQ(start_x + 0.5f, positions[0]->x_float());
  EXPECT_LT(positions[0]->y_float(), start_y + 1.5f);
  EXPECT_EQ(start_y + 1, positions[0]->grid_aligned_y());

  // Player 1 moved one cell to the left.
  EXPECT_FLOAT_EQ(start_y + 0.5f, positions[1]->y_float());
  EXPECT_GT(positions[1]->x_float(), start_x - 0.5f);
  EXPECT_EQ(start_x - 1, positions[1]->grid_aligned_x());

  // Player 2 moved one cell up.
  EXPECT_FLOAT_EQ(start_x + 0.5f, positions[2]->x_float());
  EXPECT_GT(positions[2]->y_float(), start_y - 0.5f);
  EXPECT_EQ(start_y - 1, positions[2]->grid_aligned_y());

  // Player 3 moved one cell to the right.
  EXPECT_FLOAT_EQ(start_y + 0.5f, positions[3]->y_float());
  EXPECT_LT(positions[3]->x_float(), start_x + 1.5f);
  EXPECT_EQ(start_x + 1, positions[3]->grid_aligned_x());
}
