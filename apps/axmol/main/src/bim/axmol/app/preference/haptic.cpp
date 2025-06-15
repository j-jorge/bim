// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/preference/haptic.hpp>

#include <iscool/preferences/local_preferences.hpp>

bool bim::axmol::app::haptic_feedback_enabled(
    const iscool::preferences::local_preferences& p)
{
  return p.get_value("haptic_feedback.enabled", true);
}

void bim::axmol::app::haptic_feedback_enabled(
    iscool::preferences::local_preferences& p, bool v)
{
  p.set_value("haptic_feedback.enabled", v);
}
