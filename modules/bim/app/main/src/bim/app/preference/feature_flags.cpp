// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/preference/feature_flags.hpp>

#include <bim/app/constant/game_feature_slot_count.hpp>

#include <bim/game/feature_flags.hpp>

#include <bim/to_underlying.hpp>

#include <iscool/preferences/local_preferences.hpp>

#include <array>
#include <bit>

static const std::array<std::string, bim::app::g_game_feature_slot_count>
    g_flag_in_slot_property_name = []()
{
  std::array<std::string, bim::app::g_game_feature_slot_count> result;
  const std::string_view prefix = "feature_flags.slot_";

  for (int i = 0; i != bim::app::g_game_feature_slot_count; ++i)
    {
      const std::string v = std::to_string(i);
      result[i].reserve(prefix.size() + v.size() + 1);
      result[i] = prefix;
      result[i] += v;
    }

  return result;
}();

static const std::array<std::string, bim::app::g_game_feature_slot_count>
    g_available_slot_property_name = []()
{
  std::array<std::string, bim::app::g_game_feature_slot_count> result;
  const std::string_view prefix = "feature_flags.slot_";
  const std::string_view suffix = ".available";

  for (int i = 0; i != bim::app::g_game_feature_slot_count; ++i)
    {
      const std::string v = std::to_string(i);
      result[i].reserve(prefix.size() + v.size() + suffix.size() + 1);
      result[i] = prefix;
      result[i] += v;
      result[i] += suffix;
    }

  return result;
}();

bim::game::feature_flags bim::app::available_feature_flags(
    const iscool::preferences::local_preferences& p)
{
  return (bim::game::feature_flags)p.get_value(
      "feature_flags.available", std::int64_t(bim::game::feature_flags{}));
}

void bim::app::available_feature_flags(
    iscool::preferences::local_preferences& p, bim::game::feature_flags v)
{
  p.set_value("feature_flags.available", std::int64_t(v));
}

bim::game::feature_flags
bim::app::feature_flag_in_slot(const iscool::preferences::local_preferences& p,
                               std::int64_t slot)
{
  assert(slot >= 0);
  assert(slot < g_game_feature_slot_count);
  return (bim::game::feature_flags)p.get_value(
      g_flag_in_slot_property_name[slot], (std::int64_t)0);
}

void bim::app::feature_flag_in_slot(iscool::preferences::local_preferences& p,
                                    std::int64_t slot,
                                    bim::game::feature_flags f)
{
  assert(slot >= 0);
  assert(slot < g_game_feature_slot_count);
  assert(std::popcount(std::underlying_type_t<bim::game::feature_flags>(f))
         <= 1);
  p.set_value(g_flag_in_slot_property_name[slot], (std::int64_t)f);
}

bim::game::feature_flags bim::app::enabled_feature_flags(
    const iscool::preferences::local_preferences& p)
{
  bim::game::feature_flags features{};

  for (int i = 0; i != bim::app::g_game_feature_slot_count; ++i)
    features |= feature_flag_in_slot(p, i);

  return features;
}

bool bim::app::available_feature_slot(
    const iscool::preferences::local_preferences& p, std::int64_t slot)
{
  assert(slot >= 0);
  assert(slot < g_game_feature_slot_count);
  return p.get_value(g_available_slot_property_name[slot], slot == 0);
}

void bim::app::available_feature_slot(
    iscool::preferences::local_preferences& p, std::int64_t slot, bool v)
{
  assert(slot >= 0);
  assert(slot < g_game_feature_slot_count);
  p.set_value(g_available_slot_property_name[slot], v);
}
