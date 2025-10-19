// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/preference/date_of_next_config_update.hpp>

#include <iscool/preferences/local_preferences.hpp>

std::chrono::hours bim::app::date_of_next_config_update(
    const iscool::preferences::local_preferences& p)
{
  return std::chrono::hours(
      p.get_value("date-of-next-config-update", (std::int64_t)0));
}

void bim::app::date_of_next_config_update(
    iscool::preferences::local_preferences& p, std::chrono::hours d)
{
  p.set_value("date-of-next-config-update", (std::int64_t)d.count());
}
