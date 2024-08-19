// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/game_name.hpp>
#include <bim/net/message/message_type.hpp>

#include <iscool/net/byte_array_serialization/byte_array_array_serialization.hpp>
#include <iscool/net/message/raw_message.hpp>

#include <cstdint>

namespace bim::net
{
  DECLARE_RAW_MESSAGE(new_game_request, message_type::new_game_request,
                      ((client_token)(request_token)) //
                      ((std::uint32_t)(feature_mask)) //
                      ((game_name)(name)));
}
