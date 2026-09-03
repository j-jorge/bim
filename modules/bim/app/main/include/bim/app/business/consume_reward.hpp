// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace Json
{
  class Value;
}

namespace bim::app
{
  struct consume_reward_response
  {
    std::int64_t coins;
  };

  bool from_json(consume_reward_response& response, const Json::Value& json);
}
