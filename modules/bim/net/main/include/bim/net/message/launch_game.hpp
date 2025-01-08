// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/encounter_id.hpp>
#include <bim/net/message/message_type.hpp>

#include <iscool/net/message/raw_message.hpp>

#include <cstdint>

namespace bim::net
{
  class launch_game
  {
  public:
    static iscool::net::message_type get_type();

    launch_game(client_token request_token, std::uint64_t seed,
                iscool::net::channel_id game_channel,
                std::uint32_t feature_mask, std::uint8_t player_count,
                std::uint8_t player_index, std::uint8_t brick_wall_probability,
                std::uint8_t arena_width, std::uint8_t arena_height);
    explicit launch_game(const iscool::net::byte_array& raw_content);

    void build_message(iscool::net::message& message) const;

    client_token get_request_token() const;
    std::uint64_t get_seed() const;
    iscool::net::channel_id get_game_channel() const;
    std::uint32_t get_feature_mask() const;
    std::uint8_t get_player_count() const;
    std::uint8_t get_player_index() const;
    std::uint8_t get_brick_wall_probability() const;
    std::uint8_t get_arena_width() const;
    std::uint8_t get_arena_height() const;

  private:
    client_token m_request_token;
    std::uint64_t m_seed;
    iscool::net::channel_id m_game_channel;
    std::uint32_t m_feature_mask;
    std::uint8_t m_player_count;
    std::uint8_t m_player_index;
    std::uint8_t m_brick_wall_probability;
    std::uint8_t m_arena_width;
    std::uint8_t m_arena_height;
  };
}
