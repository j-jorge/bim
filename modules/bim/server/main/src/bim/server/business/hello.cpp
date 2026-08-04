// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/business/hello.hpp>

#include <iscool/json/cast_uint.hpp>

#include <json/value.h>

bool bim::server::business::from_json(hello_response& r,
                                      const Json::Value& json)
{
  r.callback_delay =
      std::chrono::seconds(iscool::json::member_cast<std::uint32_t>(
          json, "callback_delay_seconds"));

  return true;
}
