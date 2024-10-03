// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::axmol::app
{
  bool
  haptic_feedback_enabled(const iscool::preferences::local_preferences& p);
  void haptic_feedback_enabled(iscool::preferences::local_preferences& p,
                               bool v);
}
