// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/feature_flags.hpp>
#include <bim/game/feature_flags_string.hpp>

#include <gtest/gtest.h>

TEST(bim_game_feature_flags_string, roundtrip)
{
  for (bim::game::feature_flags f : bim::game::g_all_game_feature_flags)
    {
      const std::string_view s = bim::game::to_simple_string(f);
      const std::optional<bim::game::feature_flags> o =
          bim::game::from_simple_string(s);

      ASSERT_TRUE(!!o) << "f="
                       << (std::underlying_type_t<bim::game::feature_flags>)f
                       << ", s=" << s;
      EXPECT_EQ(f, *o) << "s=" << s;
    }
}
