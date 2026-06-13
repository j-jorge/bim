// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/app/shop_intent_fwd.hpp>

namespace bim::axmol::app
{
  enum class shop_intent : std::uint8_t
  {
    user_request,
    program_request
  };
}
