// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/authentication_error_code_fwd.hpp>

namespace bim::net
{
  enum class authentication_error_code : std::uint8_t
  {
    bad_protocol = 1,
    refused = 2,
    maintenance = 3
  };
}
