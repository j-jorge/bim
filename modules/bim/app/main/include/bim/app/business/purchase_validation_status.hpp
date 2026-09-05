// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/app/business/purchase_validation_status_fwd.hpp>

namespace bim::app
{
  enum class purchase_validation_status : std::uint8_t
  {
    ok,
    duplicate,
    invalid
  };
}
