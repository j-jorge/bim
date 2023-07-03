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
