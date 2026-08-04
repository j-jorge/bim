// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <chrono>

namespace Json
{
  class Value;
}

namespace bim::server::business
{
  struct hello_response
  {
    std::chrono::seconds callback_delay;
  };

  bool from_json(hello_response& r, const Json::Value& json);
}
