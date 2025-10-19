// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/language_name_fwd.hpp>

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::app
{
  iscool::language_name
  user_language(const iscool::preferences::local_preferences& p);
  void user_language(iscool::preferences::local_preferences& p,
                     iscool::language_name language);
}
