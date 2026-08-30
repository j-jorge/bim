// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/preference/legacy.hpp>

#include <iscool/preferences/local_preferences.hpp>

bool bim::app::legacy_inventory_pushed(
    const iscool::preferences::local_preferences& p)
{
  return p.get_value("legacy_inventory_pushed", false);
}

void bim::app::legacy_inventory_pushed(
    iscool::preferences::local_preferences& p, bool v)
{
  p.set_value("legacy_inventory_pushed", v);
}
