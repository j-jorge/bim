// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/business/game_started.hpp>

#include <iscool/json/cast_int64.hpp>

#include <json/value.h>

bool bim::server::business::from_json(game_started_response& r,
                                      const Json::Value& json)
{
  r.game_id = iscool::json::member_cast<bim::net::game_id>(json, "game_id");

  return true;
}
