// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>

namespace Json
{
  class Value;
}

namespace bim::app
{
  struct update_nickname_response
  {
    std::chrono::system_clock::time_point nickname_change_allowed_date;
  };

  bool from_json(update_nickname_response& response, const Json::Value& json);
}
