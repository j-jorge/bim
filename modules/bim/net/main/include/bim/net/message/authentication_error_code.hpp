// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::net
{
  enum class authentication_error_code : std::uint8_t
  {
    bad_protocol = 1
  };
}
