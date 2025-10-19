// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::app
{
  bool music_enabled(const iscool::preferences::local_preferences& p);
  void music_enabled(iscool::preferences::local_preferences& p, bool v);

  bool effects_enabled(const iscool::preferences::local_preferences& p);
  void effects_enabled(iscool::preferences::local_preferences& p, bool v);
}
