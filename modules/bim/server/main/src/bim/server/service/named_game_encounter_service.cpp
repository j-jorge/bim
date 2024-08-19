// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/service/named_game_encounter_service.hpp>

#include <bim/server/service/game_service.hpp>
#include <bim/server/service/matchmaking_service.hpp>

#include <bim/net/message/new_game_request.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/time/now.hpp>

struct bim::server::named_game_encounter_service::encounter_info
{
  std::string name;
  iscool::signals::scoped_connection clean_up_connection;
};

bim::server::named_game_encounter_service::named_game_encounter_service(
    iscool::net::socket_stream& socket, const game_service& game_service,
    matchmaking_service& matchmaking_service)
  : m_game_service(game_service)
  , m_matchmaking_service(matchmaking_service)
{}

bim::server::named_game_encounter_service::~named_game_encounter_service() =
    default;

void bim::server::named_game_encounter_service::process(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    const bim::net::new_game_request& request)
{
  if (m_game_service.is_in_active_game(session))
    return;

  ic_log(iscool::log::nature::info(), "named_game_encounter_service",
         "New game request for session %d.", session);

  char name[std::tuple_size_v<bim::net::game_name> + 1];
  std::copy(request.get_name().begin(), request.get_name().end(), name);
  name[std::size(name) - 1] = '\0';

  const name_to_encounter_id_map::iterator it = m_encounter_ids.find(name);

  if (it == m_encounter_ids.end())
    create_encounter(endpoint, session, request, name);
  else
    update_encounter(endpoint, session, request, it->second);
}

void bim::server::named_game_encounter_service::create_encounter(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    const bim::net::new_game_request& request, std::string name)
{
  ic_log(iscool::log::nature::info(), "named_game_encounter_service",
         "Creating new encounter for game '%s' on request of session %d.",
         name, session);

  const bim::net::encounter_id encounter_id =
      m_matchmaking_service.new_encounter(endpoint, session,
                                          request.get_request_token());

  m_encounter_ids[name] = encounter_id;

  assert(m_encounters.find(encounter_id) == m_encounters.end());
  encounter_info& encounter = m_encounters[encounter_id];

  encounter.name = std::move(name);
  encounter.clean_up_connection = schedule_clean_up(encounter_id);
}

void bim::server::named_game_encounter_service::update_encounter(
    const iscool::net::endpoint& endpoint, iscool::net::session_id session,
    const bim::net::new_game_request& request,
    bim::net::encounter_id encounter_id)
{
  if (m_matchmaking_service.refresh_encounter(encounter_id, endpoint, session,
                                              request.get_request_token()))
    m_encounters[encounter_id].clean_up_connection =
        schedule_clean_up(encounter_id);
}

iscool::signals::connection
bim::server::named_game_encounter_service::schedule_clean_up(
    bim::net::encounter_id encounter_id)
{
  return iscool::schedule::delayed_call(
      [this, encounter_id]() -> void
      {
        clean_up(encounter_id);
      },
      std::chrono::minutes(3));
}

void bim::server::named_game_encounter_service::clean_up(
    bim::net::encounter_id encounter_id)
{
  ic_log(iscool::log::nature::info(), "named_game_encounter_service",
         "Cleaning up encounter %d.", encounter_id);

  const encounter_map::iterator it = m_encounters.find(encounter_id);

  m_encounter_ids.erase(it->second.name);
  m_encounters.erase(it);
}
