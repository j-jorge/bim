// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::axmol::app
{
  std::chrono::hours date_of_next_version_update_message(
      const iscool::preferences::local_preferences& p);

  void date_of_next_version_update_message(
      iscool::preferences::local_preferences& p, std::chrono::hours d);
}
