// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace Json
{
  class Value;
}

namespace bim::app
{
  class player_profile;

  bool from_json(player_profile& p, const Json::Value& json);
}
