#pragma once

#include <string>

namespace iscool::preferences
{
  class local_preferences;
}

namespace bim::axmol::app
{
  std::string
  preferred_game_server(const iscool::preferences::local_preferences& p);
}
