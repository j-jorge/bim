// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/preference/controls.hpp>

#include <iscool/preferences/local_preferences.hpp>

bool bim::axmol::app::direction_pad_on_the_left(
    const iscool::preferences::local_preferences& p)
{
  return p.get_value("controls.direction_pad_on_the_left", false);
}

void bim::axmol::app::direction_pad_on_the_left(
    iscool::preferences::local_preferences& p, bool v)
{
  p.set_value("controls.direction_pad_on_the_left", v);
}

bool bim::axmol::app::direction_pad_kind_is_stick(
    const iscool::preferences::local_preferences& p)
{
  return p.get_value("controls.direction_pad_kind_is_stick", false);
}

void bim::axmol::app::direction_pad_kind_is_stick(
    iscool::preferences::local_preferences& p, bool v)
{
  p.set_value("controls.direction_pad_kind_is_stick", v);
}
