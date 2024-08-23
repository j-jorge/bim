// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/lobby_service.hpp>

#include <bim/net/message/accept_named_game.hpp>
#include <bim/net/message/accept_random_game.hpp>
#include <bim/net/message/new_named_game_request.hpp>
#include <bim/net/message/new_random_game_request.hpp>
#include <bim/net/message/try_deserialize_message.hpp>

bim::server::lobby_service::lobby_service(iscool::net::socket_stream& socket,
                                          game_service& game_service)
  : m_named_game_encounter(socket, game_service)
  , m_random_game_encounter(socket, game_service)
{}

bim::server::lobby_service::~lobby_service() = default;

void bim::server::lobby_service::process(const iscool::net::endpoint& endpoint,
                                         const iscool::net::message& message)
{
  assert(message.get_session_id() != 0);

  if (message.get_channel_id() != 0)
    return;

  switch (message.get_type())
    {
    case bim::net::message_type::new_named_game_request:
      handle_new_named_game_request(endpoint, message);
      break;
    case bim::net::message_type::new_random_game_request:
      handle_new_random_game_request(endpoint, message);
      break;
    case bim::net::message_type::accept_named_game:
      handle_accept_named_game(endpoint, message);
      break;
    case bim::net::message_type::accept_random_game:
      handle_accept_random_game(endpoint, message);
      break;
    }
}

void bim::server::lobby_service::handle_new_named_game_request(
    const iscool::net::endpoint& endpoint, const iscool::net::message& m)

{
  const std::optional<bim::net::new_named_game_request> message =
      bim::net::try_deserialize_message<bim::net::new_named_game_request>(m);

  if (message)
    m_named_game_encounter.process(endpoint, m.get_session_id(), *message);
}

void bim::server::lobby_service::handle_new_random_game_request(
    const iscool::net::endpoint& endpoint, const iscool::net::message& m)

{
  const std::optional<bim::net::new_random_game_request> message =
      bim::net::try_deserialize_message<bim::net::new_random_game_request>(m);

  if (message)
    m_random_game_encounter.process(endpoint, m.get_session_id(), *message);
}

void bim::server::lobby_service::handle_accept_named_game(
    const iscool::net::endpoint& endpoint, const iscool::net::message& m)

{
  const std::optional<bim::net::accept_named_game> message =
      bim::net::try_deserialize_message<bim::net::accept_named_game>(m);

  if (message)
    m_named_game_encounter.mark_as_ready(endpoint, m.get_session_id(),
                                         *message);
}

void bim::server::lobby_service::handle_accept_random_game(
    const iscool::net::endpoint& endpoint, const iscool::net::message& m)

{
  const std::optional<bim::net::accept_random_game> message =
      bim::net::try_deserialize_message<bim::net::accept_random_game>(m);

  if (message)
    m_random_game_encounter.mark_as_ready(endpoint, m.get_session_id(),
                                          *message);
}
