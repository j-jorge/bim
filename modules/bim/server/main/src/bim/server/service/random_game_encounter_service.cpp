// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/random_game_encounter_service.hpp>

#include <bim/server/config.hpp>
#include <bim/server/service/game_service.hpp>

#include <bim/net/message/accept_random_game.hpp>
#include <bim/net/message/new_random_game_request.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/time/now.hpp>

bim::server::random_game_encounter_service::random_game_encounter_service(
    const config& config, iscool::net::socket_stream& socket,
    game_service& game_service)
  : m_game_service(game_service)
  , m_matchmaking_service(config, socket, game_service)
  , m_auto_start_delay(config.random_game_auto_start_delay)
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
  const bim::net::encounter_id encounter_id = message.get_encounter_id();

  ic_log(iscool::log::nature::info(), "random_game_encounter_service",
         "Accepted game. Session {}, encounter {}.", session, encounter_id);

  matchmaking_service::try_start_mode try_start_mode =
      matchmaking_service::try_start_mode::wait;

  const auto_start_date_map::const_iterator it =
      m_auto_start_date.find(encounter_id);
  const std::chrono::nanoseconds now =
      iscool::time::now<std::chrono::nanoseconds>();

  if (it == m_auto_start_date.end())
    m_auto_start_date[encounter_id] = now + m_auto_start_delay;
  else if (now >= it->second)
    {
      m_auto_start_date.erase(it);
      try_start_mode = matchmaking_service::try_start_mode::now;
    }

  m_matchmaking_service.mark_as_ready(endpoint, session, encounter_id,
                                      message.get_request_token(),
                                      message.get_features(), try_start_mode);

  clean_up();
}

void bim::server::random_game_encounter_service::clean_up()
{
  for (bim::net::encounter_id encounter_id :
       m_matchmaking_service.garbage_encounters())
    m_auto_start_date.erase(encounter_id);

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
