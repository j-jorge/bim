// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::axmol::app
{
  bool music_enabled(const iscool::preferences::local_preferences& p);
  bool effects_enabled(const iscool::preferences::local_preferences& p);
}
