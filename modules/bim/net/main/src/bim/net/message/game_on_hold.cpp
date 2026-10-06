// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/net/message/game_on_hold.hpp>

#include <bim/assume.hpp>

iscool::net::message_type bim::net::game_on_hold::get_type()
{
  return message_type::game_on_hold;
}

bim::net::game_on_hold::game_on_hold(client_token request_token,
                                     encounter_id encounter_id,
                                     std::span<const user_id> users)
  : m_request_token(request_token)
  , m_encounter_id(encounter_id)
  , m_player_count(users.size())
{
  bim_assume(m_player_count > 0);
  bim_assume(m_player_count <= bim::game::g_max_player_count);

  for (std::size_t i = 0; i != m_player_count; ++i)
    m_users[i] = users[i];
}

bim::net::game_on_hold::game_on_hold(
    const iscool::net::byte_array& raw_content)
{
  iscool::net::byte_array_reader reader(raw_content);
  reader >> m_request_token >> m_encounter_id >> m_player_count;

  if ((m_player_count == 0)
      || (m_player_count > bim::game::g_max_player_count))
    throw std::runtime_error("");

  for (std::size_t i = 0; i != m_player_count; ++i)
    {
      std::uint64_t u;
      reader >> u;
      m_users[i] = u;
    }
}

void bim::net::game_on_hold::build_message(iscool::net::message& message) const
{
  message.reset(get_type());
  iscool::net::byte_array& content = message.get_content();

  content << m_request_token << m_encounter_id << m_player_count;

  bim_assume(m_player_count > 0);
  bim_assume(m_player_count <= bim::game::g_max_player_count);

  for (std::size_t i = 0; i != m_player_count; ++i)
    content << (std::uint64_t)m_users[i];
}

bim::net::client_token bim::net::game_on_hold::get_request_token() const
{
  return m_request_token;
}

bim::net::encounter_id bim::net::game_on_hold::get_encounter_id() const
{
  return m_encounter_id;
}

std::span<const bim::net::user_id> bim::net::game_on_hold::get_users() const
{
  return std::span(m_users.data(), m_player_count);
}
