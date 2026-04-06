// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/server/service/bot_availability_fwd.hpp>

namespace bim::server
{
  enum class bot_availability : std::uint8_t
  {
    unavailable,
    available
  };
}
