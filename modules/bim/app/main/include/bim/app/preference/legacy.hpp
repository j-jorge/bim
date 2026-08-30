// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::app
{
  bool
  legacy_inventory_pushed(const iscool::preferences::local_preferences& p);
  void legacy_inventory_pushed(iscool::preferences::local_preferences& p,
                               bool v);
}
