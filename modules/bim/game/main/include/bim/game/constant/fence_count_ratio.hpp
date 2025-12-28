// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace bim::game
{
  // ((number of free cells) / g_fence_count_ratio) cells can receive a fence.
  constexpr int g_fence_count_ratio = 3;
}
