#pragma once

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::axmol::app
{
  bool
  haptic_feedback_enabled(const iscool::preferences::local_preferences& p);
}
