// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/game_id.hpp>

namespace Json
{
  class Value;
}

namespace bim::server::business
{
  struct game_started_response
  {
    bim::net::game_id game_id;
  };

  bool from_json(game_started_response& r, const Json::Value& json);
}
