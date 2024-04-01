#include <bim/server/service/random_game_encounter_service.hpp>

#include <bim/server/service/matchmaking_service.hpp>

#include <bim/net/message/new_random_game_request.hpp>

#include <iscool/log/causeless_log.hpp>
#include <iscool/log/nature/info.hpp>

bim::server::random_game_encounter_service::random_game_encounter_service(
    iscool::net::socket_stream& socket,
    matchmaking_service& matchmaking_service)
  : m_matchmaking_service(matchmaking_service)
{}

bim::server::random_game_encounter_service::~random_game_encounter_service() =
    default;

void bim::server::random_game_encounter_service::process(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    const bim::net::new_random_game_request& request)
{
  const bim::net::client_token request_token = request.get_request_token();

  if (!m_encounter_id
      || !m_matchmaking_service.refresh_encounter(*m_encounter_id, endpoint,
                                                  session, request_token))
    {
      ic_causeless_log(iscool::log::nature::info(),
                       "random_game_encounter_service",
                       "New encounter for session %d.", session);

      m_encounter_id = m_matchmaking_service.new_encounter(endpoint, session,
                                                           request_token);
    }
}
