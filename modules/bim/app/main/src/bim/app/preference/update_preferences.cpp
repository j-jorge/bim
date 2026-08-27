// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/preference/update_preferences.hpp>

#include <iscool/preferences/local_preferences.hpp>

void bim::app::update_preferences(iscool::preferences::local_preferences& p)
{
  // Since the state is stored on the business server we just cannot upgrade
  // from version zero to the current version.
  p.set_value("version", (std::int64_t)3);
}
