// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/preference/update_preferences.hpp>

#include <bim/app/config.hpp>

#include <bim/game/feature_flags.hpp>

#include <iscool/preferences/local_preferences.hpp>
#include <iscool/schedule/manual_scheduler.hpp>
#include <iscool/schedule/setup.hpp>

#include <gtest/gtest.h>

class bim_app_update_preferences : public ::testing::Test
{
public:
  bim_app_update_preferences();
  ~bim_app_update_preferences();

private:
  iscool::schedule::manual_scheduler m_scheduler;
};

bim_app_update_preferences::bim_app_update_preferences()
{
  iscool::schedule::initialize(m_scheduler.get_delayed_call_delegate());
}

bim_app_update_preferences::~bim_app_update_preferences()
{
  iscool::schedule::finalize();
}

TEST_F(bim_app_update_preferences, feature_flags_slots_from_v1)
{
  const bim::app::config config;

  const bim::game::feature_flags all_features =
      bim::game::feature_flags::falling_blocks
      | bim::game::feature_flags::fog_of_war
      | bim::game::feature_flags::invisibility
      | bim::game::feature_flags::shield;

  for (std::uint32_t mask = 0; mask <= (std::uint32_t)all_features; ++mask)
    {
      iscool::preferences::local_preferences preferences({});
      preferences.set_value("version", (std::int64_t)1);
      preferences.set_value("feature_flags.available",
                            (std::int64_t)all_features);
      preferences.set_value("feature_flags.enabled", (std::int64_t)mask);

      bim::app::update_preferences(preferences, config);

      ASSERT_TRUE(
          preferences.get_value("feature_flags.slot_0.available", false))
          << "mask=" << std::hex << mask;
      ASSERT_FALSE(
          preferences.get_value("feature_flags.slot_1.available", false))
          << "mask=" << std::hex << mask;

      if (mask == 0)
        {
          ASSERT_EQ(0, preferences.get_value("feature_flags.slot_0",
                                             (std::int64_t)0))
              << "mask=" << std::hex << mask;
        }
      else
        ASSERT_NE(
            0, preferences.get_value("feature_flags.slot_0", (std::int64_t)0)
                   & mask)
            << "mask=" << std::hex << mask;

      ASSERT_EQ(0,
                preferences.get_value("feature_flags.slot_1", (std::int64_t)0))
          << "mask=" << std::hex << mask;

      ASSERT_EQ(2, preferences.get_value("version", (std::int64_t)0));
    }
}
