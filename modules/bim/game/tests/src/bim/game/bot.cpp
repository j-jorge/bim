// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/bot.hpp>

#include <bim/game/component/player_action.hpp>
#include <bim/game/constant/default_arena_size.hpp>
#include <bim/game/constant/default_crate_probability.hpp>
#include <bim/game/constant/max_player_count.hpp>
#include <bim/game/contest.hpp>
#include <bim/game/contest_fingerprint.hpp>
#include <bim/game/contest_result.hpp>
#include <bim/game/feature_flags.hpp>
#include <bim/game/feature_flags_string.hpp>
#include <bim/game/player_action.hpp>

#include <cstdio>
#include <cstring>
#include <random>

#include <gtest/gtest.h>

static std::vector<bim::game::feature_flags> all_feature_flags_combined()
{
  std::vector<bim::game::feature_flags> result;
  result.push_back({});

  for (bim::game::feature_flags f : bim::game::g_all_game_feature_flags)
    {
      for (std::size_t i = 0, n = result.size(); i != n; ++i)
        result.push_back(result[i] | f);
    }

  return result;
}

class bim_game_bot_test
  : public testing::TestWithParam<std::tuple<int, bim::game::feature_flags>>
{
public:
  void SetUp() override;
  void TearDown() override;

protected:
  std::uint64_t m_seed;
};

void bim_game_bot_test::SetUp()
{
  const char* const e = getenv("BIM_TEST_SEED");

  if (!e)
    m_seed = std::random_device()();
  else
    {
      const char* const e_end = e + strlen(e);
      const std::from_chars_result r = std::from_chars(e, e_end, m_seed);

      ASSERT_EQ(std::errc{}, r.ec);
      ASSERT_EQ(e_end, r.ptr);
    }
}

void bim_game_bot_test::TearDown()
{
  if (HasFailure())
    printf("Seed is %lu", m_seed);
}

TEST_P(bim_game_bot_test, think)
{
  const std::uint8_t player_count = std::get<0>(GetParam());
  const bim::game::feature_flags feature_flags = std::get<1>(GetParam());

  bim::game::contest_fingerprint fingerprint = {
    .seed = m_seed,
    .features = feature_flags,
    .player_count = player_count,
    .crate_probability = bim::game::g_default_crate_probability,
    .arena_width = bim::game::g_default_arena_width,
    .arena_height = bim::game::g_default_arena_height
  };

  bim::game::contest contest(fingerprint);
  std::vector<bim::game::bot> bots;
  bots.reserve(player_count);

  for (int i = 0; i != player_count; ++i)
    bots.emplace_back(i, fingerprint.arena_width, fingerprint.arena_height,
                      fingerprint.seed);

  bim::game::contest_result result =
      bim::game::contest_result::create_still_running();
  const std::size_t max_iterations = bim::game::contest::max_game_duration
                                     / bim::game::contest::tick_interval;

  std::vector<bim::game::player_action*> actions(player_count);

  for (std::size_t i = 0; (i != max_iterations) && result.still_running(); ++i)
    {
      bim::game::collect_player_actions(actions, contest.registry());

      for (int p = 0; p != player_count; ++p)
        if (actions[p])
          *actions[p] = bots[p].think(contest);

      result = contest.tick();
    }

  EXPECT_TRUE(!result.still_running());
}

INSTANTIATE_TEST_SUITE_P(
    bim_game_bot_test_suite, bim_game_bot_test,
    testing::Combine(testing::Range(2, bim::game::g_max_player_count + 1),
                     testing::ValuesIn(all_feature_flags_combined())),
    [](const testing::TestParamInfo<bim_game_bot_test::ParamType>& info)
      {
        std::string result = std::to_string((int)std::get<0>(info.param));

        for (const bim::game::feature_flags f :
             bim::game::g_all_game_feature_flags)
          if (!!(f & std::get<1>(info.param)))
            {
              result += '_';
              result += bim::game::to_simple_string(f);
            }

        return result;
      });
