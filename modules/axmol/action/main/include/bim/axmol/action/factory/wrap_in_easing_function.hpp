#pragma once

#include <string_view>

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
}
