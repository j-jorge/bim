#include <bim/axmol/app/preference/haptic.hpp>

#include <iscool/preferences/local_preferences.hpp>

bool bim::axmol::app::haptic_feedback_enabled(
    const iscool::preferences::local_preferences& p)
{
  return p.get_value("haptic_feedback.enabled", true);
}
