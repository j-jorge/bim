// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/job/consume_game_reward_job.hpp>

#include <bim/app/analytics/coins_transaction.hpp>
#include <bim/app/analytics/error.hpp>
#include <bim/app/business/consume_reward.hpp>
#include <bim/app/business/player_profile.hpp>
#include <bim/app/business_url.hpp>

#include <bim/business/post.hpp>
#include <bim/business/request_headers.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <json/value.h>

IMPLEMENT_SIGNAL(bim::app::consume_game_reward_job, done, m_done)
IMPLEMENT_SIGNAL(bim::app::consume_game_reward_job, error, m_error)

bim::app::consume_game_reward_job::consume_game_reward_job(
    analytics_service& analytics, const bim::business::request_headers& r,
    player_profile& p)
  : m_analytics(analytics)
  , m_request_headers(r)
  , m_player_profile(p)
{}

bim::app::consume_game_reward_job::~consume_game_reward_job() = default;

void bim::app::consume_game_reward_job::start(bim::net::game_id game_id)
{
  Json::Value body;
  body["game_id"] = game_id;

  m_connection = bim::business::post<consume_reward_response>(
      BIM_BUSINESS_SERVER_URL "/client/game/consume-reward",
      m_request_headers.headers, body,
      [this](const consume_reward_response& response)
        {
          success(response);
        },
      [this, game_id]()
        {
          error(game_id);
        });
}

void bim::app::consume_game_reward_job::success(
    const consume_reward_response& response)
{
  m_player_profile.coins += response.coins;

  if (response.coins != 0)
    coins_transaction(m_analytics, "game-reward", response.coins);

  m_done(response.coins);
}

void bim::app::consume_game_reward_job::error(bim::net::game_id game_id)
{
  ic_log(iscool::log::nature::error(), "consume_game_reward_job",
         "Failed to fetch the reward for game {}.", game_id);

  bim::app::error(m_analytics, "game-reward");

  m_error();
}
