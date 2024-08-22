// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/message_type.hpp>

#include <iscool/net/message/raw_message.hpp>

#include <cstdint>

namespace bim::net
{
  DECLARE_RAW_MESSAGE(game_over, message_type::game_over,
                      ((std::uint8_t)(winner_index)));
}
