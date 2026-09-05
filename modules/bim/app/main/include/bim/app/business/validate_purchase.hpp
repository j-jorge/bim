// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/app/business/purchase_validation_status_fwd.hpp>

namespace Json
{
  class Value;
}

namespace bim::app
{
  struct validate_purchase_response
  {
    std::int64_t coins;
    purchase_validation_status status;
  };

  bool from_json(validate_purchase_response& r, const Json::Value& json);
}
