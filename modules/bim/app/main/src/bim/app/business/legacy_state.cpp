// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/business/legacy_state.hpp>

#include <iscool/json/cast.hpp>
#include <iscool/json/cast_int.hpp>

#include <json/value.h>

bool bim::app::from_json(legacy_state_transfer_response& result,
                         const Json::Value& json)
{
  switch (iscool::json::member_cast<int>(json, "transfer_state"))
    {
    case 0:
      result.transfer_state = transfer_result::disabled;
      return true;
    case 1:
      result.transfer_state = transfer_result::done;
      return true;
    case 2:
      result.transfer_state = transfer_result::already_done;
      return true;
    }

  return false;
}
