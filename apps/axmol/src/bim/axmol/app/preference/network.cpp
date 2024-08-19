// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/preference/network.hpp>

#include <iscool/preferences/local_preferences.hpp>

std::string bim::axmol::app::preferred_game_server(
    const iscool::preferences::local_preferences& p)
{
  return p.get_value("network.preferred_game_server",
                     std::string("localhost:23899"));
}
