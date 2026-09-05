// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/business/validate_purchase.hpp>

#include <iscool/json/cast.hpp>
#include <iscool/json/cast_int64.hpp>
#include <iscool/json/cast_uint8.hpp>

#include <json/value.h>

bool bim::app::from_json(validate_purchase_response& r,
                         const Json::Value& json)
{
  r.coins = iscool::json::member_cast<std::int64_t>(json, "coins");

  return true;
}
