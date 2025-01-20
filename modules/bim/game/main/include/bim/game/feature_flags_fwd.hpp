// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::game
{
  enum class feature_flags : std::uint32_t;

  feature_flags& operator|=(feature_flags& lhs, feature_flags rhs);
  feature_flags operator|(feature_flags lhs, feature_flags rhs);
  feature_flags& operator&=(feature_flags& lhs, feature_flags rhs);
  feature_flags operator&(feature_flags lhs, feature_flags rhs);
  feature_flags& operator^=(feature_flags& lhs, feature_flags rhs);
  feature_flags operator^(feature_flags lhs, feature_flags rhs);
  feature_flags operator~(feature_flags f);

  bool operator!(feature_flags f);
}
