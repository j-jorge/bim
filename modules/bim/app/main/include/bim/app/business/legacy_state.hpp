// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace Json
{
  class Value;
}

namespace bim::app
{
  enum class transfer_result : std::uint8_t
  {
    disabled,
    done,
    already_done
  };

  struct legacy_state_transfer_response
  {
    transfer_result transfer_state;
  };

  bool from_json(legacy_state_transfer_response& response,
                 const Json::Value& json);
}
