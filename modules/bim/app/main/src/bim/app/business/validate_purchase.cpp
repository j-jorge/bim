// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/business/validate_purchase.hpp>

#include <bim/app/business/purchase_validation_status.hpp>

#include <iscool/json/cast.hpp>
#include <iscool/json/cast_int64.hpp>
#include <iscool/json/cast_uint8.hpp>

#include <json/value.h>

bool bim::app::from_json(validate_purchase_response& r,
                         const Json::Value& json)
{
  switch (iscool::json::member_cast<std::uint8_t>(json, "status"))
    {
    case 0:
      r.status = purchase_validation_status::ok;
      break;
    case 1:
      r.status = purchase_validation_status::duplicate;
      break;
    case 2:
      r.status = purchase_validation_status::invalid;
      break;
    default:
      return false;
    }

  r.coins = iscool::json::member_cast<std::int64_t>(json, "coins");

  return true;
}
