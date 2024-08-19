// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/preference/audio.hpp>

#include <iscool/preferences/local_preferences.hpp>

bool bim::axmol::app::music_enabled(
    const iscool::preferences::local_preferences& p)
{
  return p.get_value("music.enabled", true);
}

bool bim::axmol::app::effects_enabled(
    const iscool::preferences::local_preferences& p)
{
  return p.get_value("effects.enabled", true);
}
