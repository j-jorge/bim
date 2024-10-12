// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::axmol::app
{
  bool
  direction_pad_on_the_left(const iscool::preferences::local_preferences& p);
  void direction_pad_on_the_left(iscool::preferences::local_preferences& p,
                                 bool v);

  bool
  direction_pad_kind_is_stick(const iscool::preferences::local_preferences& p);
  void direction_pad_kind_is_stick(iscool::preferences::local_preferences& p,
                                   bool v);
}
