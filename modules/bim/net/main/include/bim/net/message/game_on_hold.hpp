// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/encounter_id.hpp>
#include <bim/net/message/message_type.hpp>
#include <bim/net/message/user_id.hpp>

#include <bim/game/per_player_array.hpp>

#include <iscool/net/message/raw_message.hpp>

#include <span>

namespace bim::net
{
  class game_on_hold
  {
  public:
    static iscool::net::message_type get_type();

    game_on_hold(client_token request_token, encounter_id encounter_id,
                 std::span<const user_id> users);
    explicit game_on_hold(const iscool::net::byte_array& raw_content);

    void build_message(iscool::net::message& message) const;

    client_token get_request_token() const;
    encounter_id get_encounter_id() const;
    std::span<const user_id> get_users() const;

  private:
    client_token m_request_token;
    encounter_id m_encounter_id;
    bim::game::per_player_array<user_id> m_users;
    std::uint8_t m_player_count;
  };
}
