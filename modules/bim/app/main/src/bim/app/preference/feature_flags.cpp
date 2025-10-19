// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/preference/feature_flags.hpp>

#include <bim/game/feature_flags.hpp>

#include <bim/to_underlying.hpp>
#include <iscool/preferences/local_preferences.hpp>

bim::game::feature_flags bim::app::enabled_feature_flags(
    const iscool::preferences::local_preferences& p)
{
  return (bim::game::feature_flags)p.get_value(
      "feature_flags.enabled", std::int64_t(bim::game::feature_flags{}));
}

void bim::app::enabled_feature_flags(iscool::preferences::local_preferences& p,
                                     bim::game::feature_flags v)
{
  p.set_value("feature_flags.enabled", std::int64_t(v));
}

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
