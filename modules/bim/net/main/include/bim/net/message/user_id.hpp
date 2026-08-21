// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>

namespace bim::net
{
  // A user ID, as assigned by the business server.
  using user_id = std::int64_t;

  constexpr user_id not_a_user = 0;
}
