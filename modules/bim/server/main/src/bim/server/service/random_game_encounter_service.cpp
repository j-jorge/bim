// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/random_game_encounter_service.hpp>

#include <bim/server/service/game_service.hpp>

#include <bim/net/message/accept_random_game.hpp>
#include <bim/net/message/new_random_game_request.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>

bim::server::random_game_encounter_service::random_game_encounter_service(
    iscool::net::socket_stream& socket, game_service& game_service)
  : m_game_service(game_service)
  , m_matchmaking_service(socket, game_service)
{}

bim::server::random_game_encounter_service::~random_game_encounter_service() =
    default;

void bim::server::random_game_encounter_service::process(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    const bim::net::new_random_game_request& request)
{
  if (m_game_service.is_in_active_game(session))
    return;

  const bim::net::client_token request_token = request.get_request_token();

  const session_to_encounter_map::const_iterator it =
      m_session_to_encounter.find(session);

  // If the session is already matched in a existing encounter and still
  // waiting for the game to start (waiting for the "ready" message)
  if (it != m_session_to_encounter.end())
    {
      const bim::net::encounter_id encounter_id = it->second;

      if (m_matchmaking_service.refresh_encounter(encounter_id, endpoint,
                                                  session, request_token))
        return;
    }

  ic_log(iscool::log::nature::info(), "random_game_encounter_service",
         "Trying to add session {} in existing encounter.", session);

  const std::optional<bim::net::encounter_id> encounter_id =
      m_matchmaking_service.add_in_any_encounter(endpoint, session,
                                                 request_token);

  if (encounter_id)
    m_session_to_encounter[session] = *encounter_id;
  else
    m_session_to_encounter[session] =
        m_matchmaking_service.new_encounter(endpoint, session, request_token);

  clean_up();
}

void bim::server::random_game_encounter_service::mark_as_ready(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    const bim::net::accept_random_game& message)
{
  ic_log(iscool::log::nature::info(), "random_game_encounter_service",
         "Accepted game. Session {}, encounter {}.", session,
         message.get_encounter_id());

  m_matchmaking_service.mark_as_ready(endpoint, session,
                                      message.get_encounter_id(),
                                      message.get_request_token());

  clean_up();
}

void bim::server::random_game_encounter_service::clean_up()
{
  for (const bim::server::matchmaking_service::kick_session_event& event :
       m_matchmaking_service.garbage_sessions())
    {
      const session_to_encounter_map::iterator it =
          m_session_to_encounter.find(event.session);

      if ((it == m_session_to_encounter.end())
          || (event.encounter_id != it->second))
        continue;

      ic_log(iscool::log::nature::info(), "random_game_encounter_service",
             "Removing session {} from encounter {}.", event.session,
             event.encounter_id);

      m_session_to_encounter.erase(it);
    }

  m_matchmaking_service.drop_garbage();
}
