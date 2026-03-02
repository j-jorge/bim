// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>
#include <span>

namespace bim
{
  std::uint32_t crc32(std::span<const char> bytes);
}
