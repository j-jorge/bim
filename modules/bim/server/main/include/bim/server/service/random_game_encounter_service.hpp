#pragma once

#include <bim/net/message/encounter_id.hpp>

#include <iscool/net/endpoint.hpp>
#include <iscool/net/message/session_id.hpp>

#include <optional>

namespace iscool::net
{
  class socket_stream;
}

namespace bim::net
{
  class new_random_game_request;
}

namespace bim::server
{
  class game_service;
  class matchmaking_service;

  class random_game_encounter_service
  {
  public:
    random_game_encounter_service(iscool::net::socket_stream& socket,
                                  const game_service& game_service,
                                  matchmaking_service& matchmaking_service);
    ~random_game_encounter_service();

    void process(const iscool::net::endpoint& endpoint,
                 iscool::net::session_id session,
                 const bim::net::new_random_game_request& request);

  private:
    const game_service& m_game_service;
    matchmaking_service& m_matchmaking_service;

    std::optional<bim::net::encounter_id> m_encounter_id;
  };
}
