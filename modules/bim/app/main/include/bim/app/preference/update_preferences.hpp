// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::app
{
  struct config;

  void update_preferences(iscool::preferences::local_preferences& p,
                          const config& config);
}
