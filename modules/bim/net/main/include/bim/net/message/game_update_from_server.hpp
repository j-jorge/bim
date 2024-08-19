// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/message_type.hpp>

#include <bim/game/component/player_action.hpp>

#include <iscool/net/message/raw_message.hpp>

namespace bim::net
{
  class game_update_from_server
  {
  public:
    static iscool::net::message_type get_type()
    {
      return message_type::game_update_from_server;
    }

    game_update_from_server();
    explicit game_update_from_server(
        const iscool::net::byte_array& raw_content);

    iscool::net::message build_message() const;

  public:
    /** Tick from which to play the following actions apply. */
    std::uint32_t from_tick;

    /** The actual actions, per player. */
    std::vector<std::vector<bim::game::player_action>> actions;
  };
}
