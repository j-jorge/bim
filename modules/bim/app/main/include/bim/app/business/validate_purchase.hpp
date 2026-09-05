// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace Json
{
  class Value;
}

namespace bim::app
{
  struct validate_purchase_response
  {
    std::int64_t coins;
  };

  bool from_json(validate_purchase_response& r, const Json::Value& json);
}
