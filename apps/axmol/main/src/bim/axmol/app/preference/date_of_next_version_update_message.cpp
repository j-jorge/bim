// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/preference/date_of_next_version_update_message.hpp>

#include <iscool/preferences/local_preferences.hpp>

std::chrono::hours bim::axmol::app::date_of_next_version_update_message(
    const iscool::preferences::local_preferences& p)
{
  return std::chrono::hours(
      p.get_value("date-of-next-version-update-message", (std::int64_t)0));
}

void bim::axmol::app::date_of_next_version_update_message(
    iscool::preferences::local_preferences& p, std::chrono::hours d)
{
  p.set_value("date-of-next-version-update-message", (std::int64_t)d.count());
}
