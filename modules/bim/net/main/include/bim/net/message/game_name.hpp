// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/message_type.hpp>

#include <iscool/net/message/raw_message.hpp>

#include <array>

namespace bim::net
{
  using game_name = std::array<std::uint8_t, 16>;
}
