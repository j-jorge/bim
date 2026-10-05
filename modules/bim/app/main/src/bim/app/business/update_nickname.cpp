// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/business/update_nickname.hpp>

#include <iscool/json/cast.hpp>
#include <iscool/json/cast_string.hpp>
#include <iscool/time/parse_timestamp.hpp>

#include <json/value.h>

bool bim::app::from_json(update_nickname_response& response,
                         const Json::Value& json)
{
  if (!iscool::time::parse_timestamp(
          response.nickname_change_allowed_date,
          iscool::json::member_cast<std::string>(
              json, "nickname_change_allowed_date")))
    return false;

  return true;
}
