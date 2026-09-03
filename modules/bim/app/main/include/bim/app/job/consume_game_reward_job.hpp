// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/game_id.hpp>

#include <iscool/http/request_connection.hpp>
#include <iscool/signals/declare_signal.hpp>

namespace bim::business
{
  class request_headers;
}

namespace bim::app
{
  class analytics_service;
  struct consume_reward_response;
  class player_profile;

  class consume_game_reward_job
  {
    DECLARE_SIGNAL(void(std::int64_t), done, m_done)
    DECLARE_SIGNAL(void(), error, m_error)

  public:
    consume_game_reward_job(analytics_service& analytics,
                            const bim::business::request_headers& r,
                            player_profile& p);
    ~consume_game_reward_job();

    void start(bim::net::game_id game_id);

  private:
    void success(const consume_reward_response& response);
    void error(bim::net::game_id game_id);

  private:
    analytics_service& m_analytics;
    const bim::business::request_headers& m_request_headers;
    player_profile& m_player_profile;

    iscool::http::request_connection m_connection;
  };
}
