// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/business/consume_reward.hpp>

#include <iscool/json/cast.hpp>
#include <iscool/json/cast_int64.hpp>

#include <json/value.h>

bool bim::app::from_json(consume_reward_response& response,
                         const Json::Value& json)
{
  response.coins = iscool::json::member_cast<std::int64_t>(json, "coins");

  return true;
}
