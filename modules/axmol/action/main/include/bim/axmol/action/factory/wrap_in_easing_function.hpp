// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <string_view>

namespace iscool::style
{
  class declaration;
}

namespace ax
{
  class ActionEase;
  class ActionInterval;
}

namespace bim::axmol::action
{
  [[nodiscard]] ax::ActionEase*
  wrap_in_easing_function(ax::ActionInterval& action,
                          std::string_view function_name);

  [[nodiscard]] ax::ActionInterval*
  maybe_wrap_in_easing_function(ax::ActionInterval& action,
                                const iscool::style::declaration& style);
}
