// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/game/feature_flags_fwd.hpp>

#include <optional>
#include <string_view>

namespace bim::game
{
  std::string_view to_simple_string(feature_flags f);
  std::optional<feature_flags> from_simple_string(std::string_view s);
}
