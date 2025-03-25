// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::axmol::app
{
  std::chrono::hours
  date_of_next_config_update(const iscool::preferences::local_preferences& p);

  void date_of_next_config_update(iscool::preferences::local_preferences& p,
                                  std::chrono::hours d);
}
