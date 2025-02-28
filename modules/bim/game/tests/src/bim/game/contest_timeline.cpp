// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/component/bomb.hpp>
#include <bim/game/component/fractional_position_on_grid.hpp>
#include <bim/game/component/player.hpp>
#include <bim/game/component/player_action.hpp>
#include <bim/game/component/player_action_queue.hpp>
#include <bim/game/component/player_movement.hpp>
#include <bim/game/component/position_on_grid.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/contest_result.hpp>
#include <bim/game/contest_timeline.hpp>
#include <bim/game/contest_timeline_writer.hpp>
#include <bim/game/kick_event.hpp>
#include <bim/game/player_action.hpp>

#include <cstdio>

#include <gtest/gtest.h>

TEST(bim_game_contest_timeline, write_and_read)
{
  constexpr int player_count = 2;
  const bim::game::contest_fingerprint original_fingerprint{
    .seed = 0,
    .features = {},
    .player_count = player_count,
    .brick_wall_probability = 50,
    .arena_width = 5,
    .arena_height = 7
  };

  int tick_count = 0;
  int buffer_size;

  char buffer[4096];
  {
    bim::game::contest contest(original_fingerprint);

    std::FILE* const f = fmemopen(buffer, sizeof(buffer), "w");
    bim::game::contest_timeline_writer writer(f, original_fingerprint);

    std::array<bim::game::player_action*, bim::game::g_max_player_count>
        action_pointers;
    action_pointers.fill(nullptr);

    const auto tick = [&]() -> void
    {
      std::array<bim::game::player_action, player_count> actions{};

      for (int i = 0; i != player_count; ++i)
        if (action_pointers[i])
          actions[i] = *action_pointers[i];

      writer.push(contest.registry());
      ++tick_count;
      contest.tick();
    };

    for (int i = 0; i != 20; ++i)
      {
        bim::game::collect_player_actions(std::span(action_pointers),
                                          contest.registry());
        action_pointers[0]->movement = bim::game::player_movement::right;
        action_pointers[0]->drop_bomb = false;
        action_pointers[1]->movement = bim::game::player_movement::left;
        action_pointers[1]->drop_bomb = false;
        tick();
      }

    bim::game::collect_player_actions(std::span(action_pointers),
                                      contest.registry());
    action_pointers[0]->movement = bim::game::player_movement::right;
    action_pointers[0]->drop_bomb = false;
    action_pointers[1]->movement = bim::game::player_movement::left;
    action_pointers[1]->drop_bomb = true;
    tick();

    // Flush the last actions.
    for (std::size_t i = 0; i <= bim::game::player_action_queue::queue_size;
         ++i)
      {
        bim::game::collect_player_actions(std::span(action_pointers),
                                          contest.registry());
        action_pointers[0]->movement = bim::game::player_movement::idle;
        action_pointers[0]->drop_bomb = false;
        action_pointers[1]->movement = bim::game::player_movement::idle;
        action_pointers[1]->drop_bomb = false;
        tick();
      }

    buffer_size = ftell(f);
    ASSERT_LE(0, buffer_size);

    int bomb_count = 0;
    contest.registry()
        .view<bim::game::bomb, bim::game::position_on_grid>()
        .each(
            [&](const bim::game::bomb& b,
                const bim::game::position_on_grid& p) -> void
            {
              EXPECT_EQ(2, p.x);
              EXPECT_EQ(5, p.y);
              EXPECT_EQ(1, b.strength);
              EXPECT_EQ(1, b.player_index);
              ++bomb_count;
            });
    EXPECT_EQ(1, bomb_count);

    int in_game_player_count = 0;
    contest.registry()
        .view<bim::game::player, bim::game::fractional_position_on_grid>()
        .each(
            [&](const bim::game::player& p,
                const bim::game::fractional_position_on_grid& pos) -> void
            {
              ++in_game_player_count;

              if (p.index == 0)
                {
                  EXPECT_EQ(
                      bim::game::fractional_position_on_grid::value_type(2.5),
                      pos.x);
                  EXPECT_EQ(
                      bim::game::fractional_position_on_grid::value_type(1.5),
                      pos.y);
                  EXPECT_EQ(1, p.bomb_available);
                }
              else if (p.index == 1)
                {
                  EXPECT_EQ(bim::game::fractional_position_on_grid::value_type(
                                2.1875),
                            pos.x);
                  EXPECT_EQ(
                      bim::game::fractional_position_on_grid::value_type(5.5),
                      pos.y);
                  EXPECT_EQ(0, p.bomb_available);
                }
              else
                EXPECT_TRUE(false) << "p.index=" << (int)p.index;
            });
    EXPECT_EQ(2, in_game_player_count);
  }

  bim::game::contest_timeline timeline;

  std::FILE* f = fmemopen(buffer, buffer_size, "r");
  EXPECT_TRUE(bim::game::load_contest_timeline(timeline, f));
  std::fclose(f);

  EXPECT_EQ(tick_count, timeline.tick_count());

  EXPECT_EQ(original_fingerprint.seed, timeline.fingerprint().seed);
  EXPECT_EQ(original_fingerprint.brick_wall_probability,
            timeline.fingerprint().brick_wall_probability);
  EXPECT_EQ(original_fingerprint.player_count,
            timeline.fingerprint().player_count);
  EXPECT_EQ(original_fingerprint.arena_width,
            timeline.fingerprint().arena_width);
  EXPECT_EQ(original_fingerprint.arena_height,
            timeline.fingerprint().arena_height);

  bim::game::contest contest(timeline.fingerprint());

  for (int i = 0; i != tick_count; ++i)
    {
      timeline.load_tick(i, contest.registry());
      contest.tick();
    }

  // Same tests as above, the game should be in the same state.
  int bomb_count = 0;
  contest.registry().view<bim::game::bomb, bim::game::position_on_grid>().each(
      [&](const bim::game::bomb& b,
          const bim::game::position_on_grid& p) -> void
      {
        EXPECT_EQ(2, p.x);
        EXPECT_EQ(5, p.y);
        EXPECT_EQ(1, b.strength);
        EXPECT_EQ(1, b.player_index);
        ++bomb_count;
      });
  EXPECT_EQ(1, bomb_count);

  int in_game_player_count = 0;
  contest.registry()
      .view<bim::game::player, bim::game::fractional_position_on_grid>()
      .each(
          [&](const bim::game::player& p,
              const bim::game::fractional_position_on_grid& pos) -> void
          {
            ++in_game_player_count;

            if (p.index == 0)
              {
                EXPECT_EQ(
                    bim::game::fractional_position_on_grid::value_type(2.5),
                    pos.x);
                EXPECT_EQ(
                    bim::game::fractional_position_on_grid::value_type(1.5),
                    pos.y);
                EXPECT_EQ(1, p.bomb_available);
              }
            else if (p.index == 1)
              {
                EXPECT_EQ(
                    bim::game::fractional_position_on_grid::value_type(2.1875),
                    pos.x);
                EXPECT_EQ(
                    bim::game::fractional_position_on_grid::value_type(5.5),
                    pos.y);
                EXPECT_EQ(0, p.bomb_available);
              }
            else
              EXPECT_TRUE(false) << "p.index=" << (int)p.index;
          });
  EXPECT_EQ(2, in_game_player_count);
}

TEST(bim_game_contest_timeline, kick_event)
{
  constexpr int player_count = 2;
  const bim::game::contest_fingerprint original_fingerprint{
    .seed = 0,
    .features = {},
    .player_count = player_count,
    .brick_wall_probability = 50,
    .arena_width = 5,
    .arena_height = 7
  };

  int tick_count = 0;
  size_t buffer_size;

  char buffer[4096];
  {
    bim::game::contest contest(original_fingerprint);

    std::FILE* const f = fmemopen(buffer, sizeof(buffer), "w");
    bim::game::contest_timeline_writer writer(f, original_fingerprint);

    std::array<bim::game::player_action*, bim::game::g_max_player_count>
        action_pointers;

    const auto tick = [&]() -> bim::game::contest_result
    {
      std::array<bim::game::player_action, player_count> actions{};

      for (int i = 0; i != player_count; ++i)
        if (action_pointers[i])
          actions[i] = *action_pointers[i];

      writer.push(contest.registry());
      ++tick_count;
      return contest.tick();
    };

    for (int i = 0; i != 20; ++i)
      {
        bim::game::collect_player_actions(std::span(action_pointers),
                                          contest.registry());
        action_pointers[0]->movement = bim::game::player_movement::right;
        action_pointers[0]->drop_bomb = false;
        action_pointers[1]->movement = bim::game::player_movement::left;
        action_pointers[1]->drop_bomb = false;
        tick();
      }

    bim::game::kick_player(contest.registry(), 1);

    // Player 1 drops a bomb but has been kicked just above, so the bomb should
    // not be added in the game.
    bim::game::collect_player_actions(std::span(action_pointers),
                                      contest.registry());
    action_pointers[0]->movement = bim::game::player_movement::right;
    action_pointers[0]->drop_bomb = false;
    action_pointers[1]->movement = bim::game::player_movement::left;
    action_pointers[1]->drop_bomb = true;

    // One tick to kick the player out.
    const bim::game::contest_result contest_result = tick();

    // The game should be over since there is only one player left.
    EXPECT_FALSE(contest_result.still_running());
    ASSERT_TRUE(contest_result.has_a_winner());
    EXPECT_EQ(0, contest_result.winning_player());

    // Flush the last actions, it should have no effect but we want to ensure
    // that the drop bomb is not applied.
    for (std::size_t i = 0; i <= bim::game::player_action_queue::queue_size;
         ++i)
      {
        bim::game::collect_player_actions(std::span(action_pointers),
                                          contest.registry());
        action_pointers[0]->movement = bim::game::player_movement::idle;
        action_pointers[0]->drop_bomb = false;
        tick();
      }

    buffer_size = ftell(f);

    EXPECT_TRUE(contest.registry().view<bim::game::bomb>().empty());
  }

  bim::game::contest_timeline timeline;

  std::FILE* f = fmemopen(buffer, buffer_size, "r");
  EXPECT_TRUE(bim::game::load_contest_timeline(timeline, f));
  std::fclose(f);

  EXPECT_EQ(tick_count, timeline.tick_count());

  EXPECT_EQ(original_fingerprint.seed, timeline.fingerprint().seed);
  EXPECT_EQ(original_fingerprint.brick_wall_probability,
            timeline.fingerprint().brick_wall_probability);
  EXPECT_EQ(original_fingerprint.player_count,
            timeline.fingerprint().player_count);
  EXPECT_EQ(original_fingerprint.arena_width,
            timeline.fingerprint().arena_width);
  EXPECT_EQ(original_fingerprint.arena_height,
            timeline.fingerprint().arena_height);

  bim::game::contest contest(timeline.fingerprint());

  bim::game::contest_result contest_result;

  for (int i = 0; i != tick_count; ++i)
    {
      timeline.load_tick(i, contest.registry());
      contest_result = contest.tick();
    }

  // The game should be over since there is only one player left.
  EXPECT_FALSE(contest_result.still_running());
  ASSERT_TRUE(contest_result.has_a_winner());
  EXPECT_EQ(0, contest_result.winning_player());

  // Same tests as above, the game should be in the same state.
  EXPECT_TRUE(contest.registry().view<bim::game::bomb>().empty());
}

TEST(bim_game_contest_timeline, three_players_dead_or_kicked)
{
  constexpr int player_count = 3;
  const bim::game::contest_fingerprint original_fingerprint{
    .seed = 0,
    .features = {},
    .player_count = player_count,
    .brick_wall_probability = 50,
    .arena_width = 5,
    .arena_height = 7
  };

  int tick_count = 0;
  size_t buffer_size;

  char buffer[4096];
  {
    bim::game::contest contest(original_fingerprint);

    std::FILE* const f = fmemopen(buffer, sizeof(buffer), "w");
    bim::game::contest_timeline_writer writer(f, original_fingerprint);

    std::array<bim::game::player_action*, bim::game::g_max_player_count>
        action_pointers;

    const auto tick = [&]() -> bim::game::contest_result
    {
      std::array<bim::game::player_action, player_count> actions{};

      for (int i = 0; i != player_count; ++i)
        if (action_pointers[i])
          actions[i] = *action_pointers[i];

      writer.push(contest.registry());
      ++tick_count;
      return contest.tick();
    };

    for (int i = 0; i != 20; ++i)
      {
        bim::game::collect_player_actions(std::span(action_pointers),
                                          contest.registry());
        action_pointers[0]->movement = bim::game::player_movement::right;
        action_pointers[0]->drop_bomb = false;
        action_pointers[1]->movement = bim::game::player_movement::left;
        action_pointers[1]->drop_bomb = false;
        action_pointers[2]->movement = bim::game::player_movement::down;
        action_pointers[2]->drop_bomb = false;
        tick();
      }

    bim::game::kick_player(contest.registry(), 1);

    bim::game::collect_player_actions(std::span(action_pointers),
                                      contest.registry());
    action_pointers[0]->movement = bim::game::player_movement::right;
    action_pointers[0]->drop_bomb = false;
    action_pointers[2]->movement = bim::game::player_movement::idle;
    action_pointers[2]->drop_bomb = true;
    tick();

    // Flush the queue to explode the bomb.
    for (std::size_t i = 0; i <= bim::game::player_action_queue::queue_size;
         ++i)
      {
        bim::game::collect_player_actions(std::span(action_pointers),
                                          contest.registry());
        action_pointers[0]->movement = bim::game::player_movement::idle;
        action_pointers[0]->drop_bomb = false;
        action_pointers[2]->movement = bim::game::player_movement::idle;
        action_pointers[2]->drop_bomb = false;
        tick();
      }

    // Wait until the bomb explodes.
    for (std::size_t i = 0;
         i
         != (std::size_t)(std::chrono::seconds(3)
                          / bim::game::contest::tick_interval);
         ++i)
      tick();

    // One final tick, just to get the result of the contest.
    const bim::game::contest_result contest_result = tick();

    // The game should be over since there is only one player left.
    EXPECT_FALSE(contest_result.still_running());
    ASSERT_TRUE(contest_result.has_a_winner());
    EXPECT_EQ(0, contest_result.winning_player());

    buffer_size = ftell(f);
  }

  bim::game::contest_timeline timeline;

  std::FILE* f = fmemopen(buffer, buffer_size, "r");
  EXPECT_TRUE(bim::game::load_contest_timeline(timeline, f));
  std::fclose(f);

  EXPECT_EQ(tick_count, timeline.tick_count());

  EXPECT_EQ(original_fingerprint.seed, timeline.fingerprint().seed);
  EXPECT_EQ(original_fingerprint.brick_wall_probability,
            timeline.fingerprint().brick_wall_probability);
  EXPECT_EQ(original_fingerprint.player_count,
            timeline.fingerprint().player_count);
  EXPECT_EQ(original_fingerprint.arena_width,
            timeline.fingerprint().arena_width);
  EXPECT_EQ(original_fingerprint.arena_height,
            timeline.fingerprint().arena_height);

  bim::game::contest contest(timeline.fingerprint());

  bim::game::contest_result contest_result;

  for (int i = 0; i != tick_count; ++i)
    {
      timeline.load_tick(i, contest.registry());
      contest_result = contest.tick();
    }

  // The game should be over since there is only one player left.
  EXPECT_FALSE(contest_result.still_running());
  ASSERT_TRUE(contest_result.has_a_winner());
  EXPECT_EQ(0, contest_result.winning_player());
}
