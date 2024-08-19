// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/random_game_encounter_service.hpp>

#include <bim/server/service/game_service.hpp>
#include <bim/server/service/matchmaking_service.hpp>

#include <bim/net/message/new_random_game_request.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>

bim::server::random_game_encounter_service::random_game_encounter_service(
    iscool::net::socket_stream& socket, const game_service& game_service,
    matchmaking_service& matchmaking_service)
  : m_game_service(game_service)
  , m_matchmaking_service(matchmaking_service)
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

  if (!m_encounter_id
      || !m_matchmaking_service.refresh_encounter(*m_encounter_id, endpoint,
                                                  session, request_token))
    {
      ic_log(iscool::log::nature::info(), "random_game_encounter_service",
             "New encounter for session %d.", session);

      m_encounter_id = m_matchmaking_service.new_encounter(endpoint, session,
                                                           request_token);
    }
}
