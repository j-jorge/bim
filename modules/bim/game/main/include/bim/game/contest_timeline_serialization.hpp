// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstddef>
#include <cstdint>

namespace bim::game
{
  namespace contest_timeline_serialization
  {
    constexpr const char magic[] = { 'B', 'I', 'M', '!' };
    constexpr std::size_t magic_length = sizeof(magic);
    constexpr std::uint32_t file_version = 3;
  }
}
