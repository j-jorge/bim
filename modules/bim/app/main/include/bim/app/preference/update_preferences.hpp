// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::app
{
  void update_preferences(iscool::preferences::local_preferences& p);
}
