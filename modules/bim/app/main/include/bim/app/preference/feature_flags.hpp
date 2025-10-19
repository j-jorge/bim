// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/feature_flags_fwd.hpp>

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::app
{
  bim::game::feature_flags
  enabled_feature_flags(const iscool::preferences::local_preferences& p);
  void enabled_feature_flags(iscool::preferences::local_preferences& p,
                             bim::game::feature_flags v);

  bim::game::feature_flags
  available_feature_flags(const iscool::preferences::local_preferences& p);
  void available_feature_flags(iscool::preferences::local_preferences& p,
                               bim::game::feature_flags v);
}
