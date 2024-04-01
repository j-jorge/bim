#include <bim/server/service/lobby_service.hpp>

#include <bim/net/message/accept_game.hpp>
#include <bim/net/message/new_game_request.hpp>
#include <bim/net/message/new_random_game_request.hpp>

bim::server::lobby_service::lobby_service(iscool::net::socket_stream& socket,
                                          game_service& game_service)
  : m_matchmaking_service(socket, game_service)
  , m_named_game_encounter(socket, m_matchmaking_service)
  , m_random_game_encounter(socket, m_matchmaking_service)
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
    case bim::net::message_type::new_game_request:
      m_named_game_encounter.process(
          endpoint, message.get_session_id(),
          bim::net::new_game_request(message.get_content()));
      break;
    case bim::net::message_type::new_random_game_request:
      m_random_game_encounter.process(
          endpoint, message.get_session_id(),
          bim::net::new_random_game_request(message.get_content()));
      break;
    case bim::net::message_type::accept_game:
      m_matchmaking_service.mark_as_ready(
          endpoint, message.get_session_id(),
          bim::net::accept_game(message.get_content()));
      break;
    }
}
