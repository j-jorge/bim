// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/style/scale_mode_fwd.hpp>

namespace bim::axmol::style
{
  enum class scale_mode : std::uint8_t
  {
    fit,
    cover,
    device
  };
}
