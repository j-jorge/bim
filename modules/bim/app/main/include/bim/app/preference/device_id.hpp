// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <string>

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::app
{
  std::string device_id(const iscool::preferences::local_preferences& p);
  void ensure_device_id_exists(iscool::preferences::local_preferences& p);
}
